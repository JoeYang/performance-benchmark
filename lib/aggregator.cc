#include "lib/aggregator.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <thread>
#include <vector>

namespace trading {

AggregationResult aggregate_positions_single(
    const std::vector<Position>& positions) {

    AggregationResult result;
    result.total_long_exposure = 0.0;
    result.total_short_exposure = 0.0;
    result.net_exposure = 0.0;
    result.total_positions = static_cast<int>(positions.size());

    for (const auto& pos : positions) {
        double notional = pos.quantity * pos.price;

        auto& exposure = result.by_symbol[pos.symbol];
        exposure.quantity += pos.quantity;
        exposure.notional += notional;
        exposure.position_count++;

        if (exposure.quantity != 0.0) {
            exposure.avg_price = exposure.notional / exposure.quantity;
        }

        if (notional > 0) {
            result.total_long_exposure += notional;
        } else {
            result.total_short_exposure += std::abs(notional);
        }
        result.net_exposure += notional;
    }

    return result;
}

AggregationResult aggregate_positions_multi(
    const std::vector<Position>& positions,
    int num_threads) {

    std::vector<AggregationResult> partial_results(num_threads);
    std::vector<std::thread> threads;

    size_t chunk_size = (positions.size() + num_threads - 1) / num_threads;

    auto worker = [&](int thread_id) {
        size_t start = thread_id * chunk_size;
        size_t end = std::min(start + chunk_size, positions.size());

        AggregationResult& local = partial_results[thread_id];
        local.total_long_exposure = 0.0;
        local.total_short_exposure = 0.0;
        local.net_exposure = 0.0;
        local.total_positions = 0;

        for (size_t i = start; i < end; ++i) {
            const auto& pos = positions[i];
            double notional = pos.quantity * pos.price;

            auto& exposure = local.by_symbol[pos.symbol];
            exposure.quantity += pos.quantity;
            exposure.notional += notional;
            exposure.position_count++;

            if (notional > 0) {
                local.total_long_exposure += notional;
            } else {
                local.total_short_exposure += std::abs(notional);
            }
            local.net_exposure += notional;
            local.total_positions++;
        }
    };

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(worker, t);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    AggregationResult final_result;
    final_result.total_long_exposure = 0.0;
    final_result.total_short_exposure = 0.0;
    final_result.net_exposure = 0.0;
    final_result.total_positions = 0;

    for (const auto& partial : partial_results) {
        for (const auto& [symbol, exp] : partial.by_symbol) {
            auto& final_exp = final_result.by_symbol[symbol];
            final_exp.quantity += exp.quantity;
            final_exp.notional += exp.notional;
            final_exp.position_count += exp.position_count;
        }
        final_result.total_long_exposure += partial.total_long_exposure;
        final_result.total_short_exposure += partial.total_short_exposure;
        final_result.net_exposure += partial.net_exposure;
        final_result.total_positions += partial.total_positions;
    }

    for (auto& [symbol, exp] : final_result.by_symbol) {
        if (exp.quantity != 0.0) {
            exp.avg_price = exp.notional / exp.quantity;
        }
    }

    return final_result;
}

std::vector<std::pair<std::string, NetExposure>> get_top_exposures(
    const AggregationResult& result, size_t top_n) {

    std::vector<std::pair<std::string, NetExposure>> exposures(
        result.by_symbol.begin(), result.by_symbol.end());

    std::sort(exposures.begin(), exposures.end(),
        [](const auto& a, const auto& b) {
            return std::abs(a.second.notional) > std::abs(b.second.notional);
        });

    if (exposures.size() > top_n) {
        exposures.resize(top_n);
    }

    return exposures;
}

}  // namespace trading
