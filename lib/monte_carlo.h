#ifndef LIB_MONTE_CARLO_H_
#define LIB_MONTE_CARLO_H_

#include "lib/position.h"

#include <vector>

namespace trading {

struct VaRResult {
    double var_95;
    double var_99;
    double expected_shortfall;
    double mean_pnl;
    double std_pnl;
};

std::vector<double> simulate_portfolio_pnl(
    const std::vector<Position>& positions,
    size_t num_simulations,
    double time_horizon,
    unsigned int seed);

VaRResult calculate_var(const std::vector<double>& pnl_values);

VaRResult run_monte_carlo_single(
    const std::vector<Position>& positions,
    size_t num_simulations,
    double time_horizon,
    unsigned int seed = 42);

VaRResult run_monte_carlo_multi(
    const std::vector<Position>& positions,
    size_t num_simulations,
    double time_horizon,
    int num_threads,
    unsigned int seed = 42);

}  // namespace trading

#endif  // LIB_MONTE_CARLO_H_
