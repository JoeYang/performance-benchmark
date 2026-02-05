#ifndef LIB_AGGREGATOR_H_
#define LIB_AGGREGATOR_H_

#include "lib/position.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace trading {

struct NetExposure {
    double quantity;
    double notional;
    double avg_price;
    int position_count;
};

struct AggregationResult {
    std::unordered_map<std::string, NetExposure> by_symbol;
    double total_long_exposure;
    double total_short_exposure;
    double net_exposure;
    int total_positions;
};

AggregationResult aggregate_positions_single(
    const std::vector<Position>& positions);

AggregationResult aggregate_positions_multi(
    const std::vector<Position>& positions,
    int num_threads);

std::vector<std::pair<std::string, NetExposure>> get_top_exposures(
    const AggregationResult& result, size_t top_n);

}  // namespace trading

#endif  // LIB_AGGREGATOR_H_
