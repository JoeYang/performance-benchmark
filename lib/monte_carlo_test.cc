#include "lib/monte_carlo.h"

#include <gtest/gtest.h>
#include <cmath>

namespace trading {
namespace {

TEST(MonteCarloTest, SimulatePnL) {
    auto positions = generate_random_positions(10, 42);
    auto pnl = simulate_portfolio_pnl(positions, 1000, 1.0/252.0, 42);

    ASSERT_EQ(pnl.size(), 1000);
}

TEST(MonteCarloTest, CalculateVaR) {
    std::vector<double> pnl;
    for (int i = -500; i < 500; ++i) {
        pnl.push_back(static_cast<double>(i));
    }

    auto result = calculate_var(pnl);

    EXPECT_GT(result.var_95, 0.0);
    EXPECT_GT(result.var_99, result.var_95);
    EXPECT_NEAR(result.mean_pnl, -0.5, 1.0);
}

TEST(MonteCarloTest, SingleThreadedVaR) {
    auto positions = generate_random_positions(100, 42);
    auto result = run_monte_carlo_single(positions, 10000, 1.0/252.0, 42);

    EXPECT_GT(result.var_95, 0.0);
    EXPECT_GT(result.var_99, 0.0);
    EXPECT_GT(result.expected_shortfall, result.var_99);
}

TEST(MonteCarloTest, MultiThreadedVaR) {
    auto positions = generate_random_positions(100, 42);
    auto result = run_monte_carlo_multi(positions, 10000, 1.0/252.0, 4, 42);

    EXPECT_GT(result.var_95, 0.0);
    EXPECT_GT(result.var_99, 0.0);
}

TEST(MonteCarloTest, DeterministicSingleThread) {
    auto positions = generate_random_positions(50, 42);

    auto result1 = run_monte_carlo_single(positions, 5000, 1.0/252.0, 123);
    auto result2 = run_monte_carlo_single(positions, 5000, 1.0/252.0, 123);

    EXPECT_EQ(result1.var_95, result2.var_95);
    EXPECT_EQ(result1.var_99, result2.var_99);
}

}  // namespace
}  // namespace trading
