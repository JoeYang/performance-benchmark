#include "lib/position.h"

#include <random>

namespace trading {

std::vector<Position> generate_random_positions(size_t count, unsigned int seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> price_dist(10.0, 500.0);
    std::uniform_real_distribution<double> qty_dist(-1000.0, 1000.0);
    std::uniform_real_distribution<double> vol_dist(0.1, 0.8);
    std::uniform_real_distribution<double> expiry_dist(0.1, 2.0);
    std::uniform_int_distribution<int> type_dist(0, 2);
    std::uniform_int_distribution<int> symbol_dist(0, 499);

    std::vector<Position> positions;
    positions.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        Position pos;
        pos.symbol = "SYM" + std::to_string(symbol_dist(rng));
        pos.price = price_dist(rng);
        pos.quantity = qty_dist(rng);
        pos.volatility = vol_dist(rng);
        pos.type = static_cast<PositionType>(type_dist(rng));
        pos.risk_free_rate = 0.05;

        if (pos.type != PositionType::STOCK) {
            std::uniform_real_distribution<double> strike_dist(
                pos.price * 0.8, pos.price * 1.2);
            pos.strike = strike_dist(rng);
            pos.time_to_expiry = expiry_dist(rng);
        } else {
            pos.strike = 0.0;
            pos.time_to_expiry = 0.0;
        }

        positions.push_back(pos);
    }

    return positions;
}

double position_market_value(const Position& pos) {
    return pos.quantity * pos.price;
}

}  // namespace trading
