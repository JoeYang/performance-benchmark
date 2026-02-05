#include "lib/greeks.h"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <future>
#include <thread>
#include <vector>

// Fallback definitions for portability
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2 0.70710678118654752440
#endif

namespace trading {

double normal_cdf(double x) {
    return 0.5 * std::erfc(-x * M_SQRT1_2);
}

double normal_pdf(double x) {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * M_PI);
}

double black_scholes_price(double spot, double strike, double vol,
                           double rate, double time, bool is_call) {
    if (time <= 0.0 || vol <= 0.0) {
        double intrinsic = is_call ? std::max(spot - strike, 0.0)
                                   : std::max(strike - spot, 0.0);
        return intrinsic;
    }

    double d1 = (std::log(spot / strike) + (rate + 0.5 * vol * vol) * time) /
                (vol * std::sqrt(time));
    double d2 = d1 - vol * std::sqrt(time);

    if (is_call) {
        return spot * normal_cdf(d1) -
               strike * std::exp(-rate * time) * normal_cdf(d2);
    } else {
        return strike * std::exp(-rate * time) * normal_cdf(-d2) -
               spot * normal_cdf(-d1);
    }
}

Greeks calculate_greeks(const Position& pos, double bump_size) {
    Greeks result{};

    if (pos.type == PositionType::STOCK) {
        result.price = pos.price;
        result.delta = 1.0;
        result.gamma = 0.0;
        result.vega = 0.0;
        result.theta = 0.0;
        return result;
    }

    bool is_call = (pos.type == PositionType::OPTION_CALL);
    double spot = pos.price;
    double strike = pos.strike;
    double vol = pos.volatility;
    double rate = pos.risk_free_rate;
    double time = pos.time_to_expiry;

    result.price = black_scholes_price(spot, strike, vol, rate, time, is_call);

    double spot_up = spot * (1.0 + bump_size);
    double spot_down = spot * (1.0 - bump_size);
    double price_up = black_scholes_price(spot_up, strike, vol, rate, time, is_call);
    double price_down = black_scholes_price(spot_down, strike, vol, rate, time, is_call);

    result.delta = (price_up - price_down) / (spot_up - spot_down);
    result.gamma = (price_up - 2.0 * result.price + price_down) /
                   ((spot * bump_size) * (spot * bump_size));

    double vol_up = vol + bump_size;
    double price_vol_up = black_scholes_price(spot, strike, vol_up, rate, time, is_call);
    result.vega = (price_vol_up - result.price) / bump_size;

    double time_down = std::max(time - 1.0/365.0, 0.001);
    double price_time_down = black_scholes_price(spot, strike, vol, rate, time_down, is_call);
    result.theta = (price_time_down - result.price) * 365.0;

    return result;
}

std::vector<Greeks> calculate_all_greeks_single(
    const std::vector<Position>& positions, double bump_size) {
    std::vector<Greeks> results;
    results.reserve(positions.size());

    for (const auto& pos : positions) {
        results.push_back(calculate_greeks(pos, bump_size));
    }

    return results;
}

std::vector<Greeks> calculate_all_greeks_multi(
    const std::vector<Position>& positions, int num_threads,
    double bump_size) {
    std::vector<Greeks> results(positions.size());

    auto worker = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            results[i] = calculate_greeks(positions[i], bump_size);
        }
    };

    std::vector<std::thread> threads;
    size_t chunk_size = (positions.size() + num_threads - 1) / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        size_t start = t * chunk_size;
        size_t end = std::min(start + chunk_size, positions.size());
        if (start < end) {
            threads.emplace_back(worker, start, end);
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return results;
}

double total_portfolio_delta(const std::vector<Greeks>& greeks,
                             const std::vector<Position>& positions) {
    double total = 0.0;
    for (size_t i = 0; i < greeks.size(); ++i) {
        total += greeks[i].delta * positions[i].quantity;
    }
    return total;
}

}  // namespace trading
