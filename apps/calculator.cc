#include <iostream>

#include "lib/math.h"

int main() {
    std::cout << "Calculator Demo" << std::endl;
    std::cout << "2 + 3 = " << math::add(2, 3) << std::endl;
    std::cout << "4 * 5 = " << math::multiply(4, 5) << std::endl;
    return 0;
}
