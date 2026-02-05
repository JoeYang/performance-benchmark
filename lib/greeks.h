#ifndef LIB_GREEKS_H_
#define LIB_GREEKS_H_

#include "lib/position.h"

#include <vector>

namespace trading {

struct Greeks {
    double price;
    double delta;
    double gamma;
    double vega;
    double theta;
};

double normal_cdf(double x);
double normal_pdf(double x);

double black_scholes_price(double spot, double strike, double vol,
                           double rate, double time, bool is_call);

Greeks calculate_greeks(const Position& pos, double bump_size = 0.01);

std::vector<Greeks> calculate_all_greeks_single(
    const std::vector<Position>& positions, double bump_size = 0.01);

std::vector<Greeks> calculate_all_greeks_multi(
    const std::vector<Position>& positions, int num_threads,
    double bump_size = 0.01);

double total_portfolio_delta(const std::vector<Greeks>& greeks,
                             const std::vector<Position>& positions);

}  // namespace trading

#endif  // LIB_GREEKS_H_
