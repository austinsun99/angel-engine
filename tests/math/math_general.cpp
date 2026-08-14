#include <cmath>
#include "../angel_test.hpp"

#include "math/angel_math.hpp"

using namespace angel::math;

TEST_FUNC(math_general) {
    TEST_ASSERT(fac(0) == 1);
    TEST_ASSERT(fac(1) == 1);
    TEST_ASSERT(fac(2) == 2);
    TEST_ASSERT(fac(3) == 6);
    TEST_ASSERT(fac(12) == 479001600);

    TEST_ASSERT(pow(36, 0) == 1);
    TEST_ASSERT(pow(36, 1) == 36);
    TEST_ASSERT(pow(5, 2) == 25);
    TEST_ASSERT(pow(5, 3) == 125);
    TEST_ASSERT(pow(5, 4) == 625);
    TEST_ASSERT(pow(2.5, 2) == 6.25);
    return TEST_SUCCESS;
}
