#include "lib/greeks.h"

#include <gtest/gtest.h>
#include <cmath>

namespace trading {
namespace {

TEST(GreeksTest, NormalCDF) {
    EXPECT_NEAR(normal_cdf(0.0), 0.5, 1e-6);
    EXPECT_NEAR(normal_cdf(1.96), 0.975, 0.001);
    EXPECT_NEAR(normal_cdf(-1.96), 0.025, 0.001);
}

TEST(GreeksTest, BlackScholesCallPrice) {
    double price = black_scholes_price(100.0, 100.0, 0.2, 0.05, 1.0, true);
    EXPECT_GT(price, 0.0);
    EXPECT_LT(price, 100.0);
    EXPECT_NEAR(price, 10.45, 0.5);
}

TEST(GreeksTest, BlackScholesPutPrice) {
    double price = black_scholes_price(100.0, 100.0, 0.2, 0.05, 1.0, false);
    EXPECT_GT(price, 0.0);
    EXPECT_LT(price, 100.0);
}

TEST(GreeksTest, StockGreeks) {
    Position stock;
    stock.symbol = "AAPL";
    stock.quantity = 100;
    stock.price = 150.0;
    stock.volatility = 0.3;
    stock.type = PositionType::STOCK;

    Greeks g = calculate_greeks(stock);
    EXPECT_EQ(g.delta, 1.0);
    EXPECT_EQ(g.gamma, 0.0);
    EXPECT_EQ(g.vega, 0.0);
}

TEST(GreeksTest, OptionGreeks) {
    Position option;
    option.symbol = "AAPL";
    option.quantity = 10;
    option.price = 150.0;
    option.volatility = 0.3;
    option.type = PositionType::OPTION_CALL;
    option.strike = 155.0;
    option.time_to_expiry = 0.5;
    option.risk_free_rate = 0.05;

    Greeks g = calculate_greeks(option);
    EXPECT_GT(g.price, 0.0);
    EXPECT_GT(g.delta, 0.0);
    EXPECT_LT(g.delta, 1.0);
    EXPECT_GT(g.gamma, 0.0);
    EXPECT_GT(g.vega, 0.0);
}

TEST(GreeksTest, MultiThreadedConsistency) {
    auto positions = generate_random_positions(100, 123);

    auto single_result = calculate_all_greeks_single(positions);
    auto multi_result = calculate_all_greeks_multi(positions, 4);

    ASSERT_EQ(single_result.size(), multi_result.size());

    for (size_t i = 0; i < single_result.size(); ++i) {
        EXPECT_NEAR(single_result[i].delta, multi_result[i].delta, 1e-9);
        EXPECT_NEAR(single_result[i].gamma, multi_result[i].gamma, 1e-9);
    }
}

}  // namespace
}  // namespace trading
