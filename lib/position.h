#ifndef LIB_POSITION_H_
#define LIB_POSITION_H_

#include <string>
#include <vector>

namespace trading {

enum class PositionType {
    STOCK,
    OPTION_CALL,
    OPTION_PUT
};

struct Position {
    std::string symbol;
    double quantity;
    double price;
    double volatility;
    PositionType type;
    double strike;
    double time_to_expiry;
    double risk_free_rate;
};

std::vector<Position> generate_random_positions(size_t count, unsigned int seed = 42);

double position_market_value(const Position& pos);

}  // namespace trading

#endif  // LIB_POSITION_H_
