#include "lib/aggregator.h"

#include <gtest/gtest.h>
#include <cmath>

namespace trading {
namespace {

TEST(AggregatorTest, SinglePosition) {
    std::vector<Position> positions = {{
        .symbol = "AAPL",
        .quantity = 100,
        .price = 150.0,
        .volatility = 0.3,
        .type = PositionType::STOCK,
        .strike = 0.0,
        .time_to_expiry = 0.0,
        .risk_free_rate = 0.05
    }};

    auto result = aggregate_positions_single(positions);

    EXPECT_EQ(result.total_positions, 1);
    EXPECT_EQ(result.by_symbol.size(), 1);
    EXPECT_NEAR(result.total_long_exposure, 15000.0, 0.01);
    EXPECT_NEAR(result.net_exposure, 15000.0, 0.01);
}

TEST(AggregatorTest, NetExposure) {
    std::vector<Position> positions = {
        {.symbol = "AAPL", .quantity = 100, .price = 150.0, .volatility = 0.3,
         .type = PositionType::STOCK, .strike = 0, .time_to_expiry = 0, .risk_free_rate = 0.05},
        {.symbol = "AAPL", .quantity = -50, .price = 150.0, .volatility = 0.3,
         .type = PositionType::STOCK, .strike = 0, .time_to_expiry = 0, .risk_free_rate = 0.05}
    };

    auto result = aggregate_positions_single(positions);

    EXPECT_EQ(result.by_symbol.size(), 1);
    EXPECT_NEAR(result.by_symbol["AAPL"].quantity, 50.0, 0.01);
}

TEST(AggregatorTest, MultiThreadedConsistency) {
    auto positions = generate_random_positions(1000, 42);

    auto single_result = aggregate_positions_single(positions);
    auto multi_result = aggregate_positions_multi(positions, 4);

    EXPECT_EQ(single_result.by_symbol.size(), multi_result.by_symbol.size());
    EXPECT_NEAR(single_result.net_exposure, multi_result.net_exposure, 0.01);
    EXPECT_NEAR(single_result.total_long_exposure,
                multi_result.total_long_exposure, 0.01);
}

TEST(AggregatorTest, TopExposures) {
    auto positions = generate_random_positions(500, 42);
    auto result = aggregate_positions_single(positions);
    auto top = get_top_exposures(result, 10);

    EXPECT_LE(top.size(), 10);

    for (size_t i = 1; i < top.size(); ++i) {
        EXPECT_GE(std::abs(top[i-1].second.notional),
                  std::abs(top[i].second.notional));
    }
}

}  // namespace
}  // namespace trading
