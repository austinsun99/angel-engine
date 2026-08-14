#include "../angel_test.hpp"

#include "math/vector3.h"

using namespace angel::math;

TEST_FUNC(vector2_arithmetic) {
    Vector3 vec1;
    TEST_ASSERT(vec1.x == 0 && vec1.y == 0 && vec1.z == 0);

    const Vector3 vec2 = Vector3(2, 3, 4);
    TEST_ASSERT(vec2.x == 2 && vec2.y == 3 && vec2.z == 4)

    vec1 += vec2;
    TEST_ASSERT(vec1 == vec2);
    TEST_ASSERT(vec1 == Vector3(2, 3, 4));

    vec1 += vec2;
    TEST_ASSERT(vec1 == vec2 * 2);
    TEST_ASSERT(vec1 == Vector3(4, 6, 8));

    vec1 -= vec2;
    TEST_ASSERT(vec1 == vec2);

    vec1 /= Vector3(2, 1, 2);
    TEST_ASSERT(vec1 == Vector3(1, 3, 2));
    TEST_SUCCESS;
}
