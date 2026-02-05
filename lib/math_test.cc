#include "lib/math.h"

#include <gtest/gtest.h>

TEST(MathTest, Add) {
    EXPECT_EQ(math::add(2, 3), 5);
    EXPECT_EQ(math::add(-1, 1), 0);
}

TEST(MathTest, Multiply) {
    EXPECT_EQ(math::multiply(2, 3), 6);
    EXPECT_EQ(math::multiply(-2, 3), -6);
}
