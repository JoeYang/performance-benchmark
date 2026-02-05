#include "lib/monte_carlo.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

namespace trading {

std::vector<double> simulate_portfolio_pnl(
    const std::vector<Position>& positions,
    size_t num_simulations,
    double time_horizon,
    unsigned int seed) {

    std::mt19937 rng(seed);
    std::normal_distribution<double> normal(0.0, 1.0);

    std::vector<double> pnl_values(num_simulations);

    for (size_t sim = 0; sim < num_simulations; ++sim) {
        double portfolio_pnl = 0.0;

        for (const auto& pos : positions) {
            double z = normal(rng);
            double drift = (pos.risk_free_rate - 0.5 * pos.volatility * pos.volatility) * time_horizon;
            double diffusion = pos.volatility * std::sqrt(time_horizon) * z;
            double price_change_factor = std::exp(drift + diffusion);
            double new_price = pos.price * price_change_factor;
            double position_pnl = pos.quantity * (new_price - pos.price);
            portfolio_pnl += position_pnl;
        }

        pnl_values[sim] = portfolio_pnl;
    }

    return pnl_values;
}

VaRResult calculate_var(const std::vector<double>& pnl_values) {
    if (pnl_values.empty()) {
        return {0.0, 0.0, 0.0, 0.0, 0.0};
    }

    std::vector<double> sorted_pnl = pnl_values;
    std::sort(sorted_pnl.begin(), sorted_pnl.end());

    size_t n = sorted_pnl.size();
    size_t idx_95 = static_cast<size_t>(n * 0.05);
    size_t idx_99 = static_cast<size_t>(n * 0.01);

    VaRResult result;
    result.var_95 = -sorted_pnl[idx_95];
    result.var_99 = -sorted_pnl[idx_99];

    double es_sum = 0.0;
    for (size_t i = 0; i <= idx_99; ++i) {
        es_sum += sorted_pnl[i];
    }
    result.expected_shortfall = -es_sum / (idx_99 + 1);

    double sum = std::accumulate(pnl_values.begin(), pnl_values.end(), 0.0);
    result.mean_pnl = sum / n;

    double sq_sum = 0.0;
    for (double pnl : pnl_values) {
        sq_sum += (pnl - result.mean_pnl) * (pnl - result.mean_pnl);
    }
    result.std_pnl = std::sqrt(sq_sum / n);

    return result;
}

VaRResult run_monte_carlo_single(
    const std::vector<Position>& positions,
    size_t num_simulations,
    double time_horizon,
    unsigned int seed) {

    auto pnl_values = simulate_portfolio_pnl(positions, num_simulations,
                                              time_horizon, seed);
    return calculate_var(pnl_values);
}

VaRResult run_monte_carlo_multi(
    const std::vector<Position>& positions,
    size_t num_simulations,
    double time_horizon,
    int num_threads,
    unsigned int seed) {

    std::vector<std::vector<double>> thread_results(num_threads);
    std::vector<std::thread> threads;

    size_t sims_per_thread = num_simulations / num_threads;
    size_t remainder = num_simulations % num_threads;

    auto worker = [&](int thread_id, size_t num_sims) {
        unsigned int thread_seed = seed + thread_id * 12345;
        thread_results[thread_id] = simulate_portfolio_pnl(
            positions, num_sims, time_horizon, thread_seed);
    };

    for (int t = 0; t < num_threads; ++t) {
        size_t sims = sims_per_thread + (t < static_cast<int>(remainder) ? 1 : 0);
        threads.emplace_back(worker, t, sims);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::vector<double> all_pnl;
    all_pnl.reserve(num_simulations);
    for (const auto& result : thread_results) {
        all_pnl.insert(all_pnl.end(), result.begin(), result.end());
    }

    return calculate_var(all_pnl);
}

}  // namespace trading
