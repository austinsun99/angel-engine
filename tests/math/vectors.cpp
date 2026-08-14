#include "../angel_test.hpp"

#include "math/vectors/angel_vectors.hpp"

using namespace angel::math;

TEST_FUNC(vector2_generic) {
    Vector2 vec1;
    TEST_ASSERT(vec1.x == 0 && vec1.y == 0);

    const Vector2 vec2 = Vector2(2, 3);
    TEST_ASSERT(vec2.x == 2 && vec2.y == 3)

    vec1 += vec2;
    TEST_ASSERT(vec1 == vec2);
    TEST_ASSERT(vec1 == Vector2(2, 3));

    vec1 += vec2;
    TEST_ASSERT(vec1 == vec2 * 2);
    TEST_ASSERT(vec1 == Vector2(4, 6));

    vec1 -= vec2;
    TEST_ASSERT(vec1 == vec2);

    vec1 /= Vector2(2, 1);
    TEST_ASSERT(vec1 == Vector2(1, 3));
    vec1 *= Vector2(2, 1);
    TEST_ASSERT(vec1 == vec2);

    return TEST_SUCCESS;
};

TEST_FUNC(vector3_generic) {
    Vector3 vec1;
    TEST_ASSERT(vec1.x == 0 && vec1.y == 0 && vec1.z == 0);

    const Vector3 vec2 = Vector3(2, 3, 4);
    TEST_ASSERT(vec2.x == 2 && vec2.y == 3 && vec2.z == 4)
    TEST_ASSERT(vec2.r == 2 && vec2.g == 3 && vec2.b == 4)

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
    vec1 *= Vector3(2, 1, 2);
    TEST_ASSERT(vec1 == vec2);

    return TEST_SUCCESS;
}

TEST_FUNC(vector4_generic) {
    Vector4 vec1;
    TEST_ASSERT(vec1.x == 0 && vec1.y == 0 && vec1.z == 0 && vec1.w == 0);

    const Vector4 vec2 = Vector4(2, 3, 4, 6);
    TEST_ASSERT(vec2.x == 2 && vec2.y == 3 && vec2.z == 4 && vec2.w == 6)
    TEST_ASSERT(vec2.r == 2 && vec2.g == 3 && vec2.b == 4 && vec2.a == 6)

    vec1 += vec2;
    TEST_ASSERT(vec1 == vec2);
    TEST_ASSERT(vec1 == Vector4(2, 3, 4, 6));

    vec1 += vec2;
    TEST_ASSERT(vec1 == vec2 * 2);
    TEST_ASSERT(vec1 == Vector4(4, 6, 8, 12));

    vec1 -= vec2;
    TEST_ASSERT(vec1 == vec2);

    vec1 /= Vector4(2, 1, 2, 4);
    TEST_ASSERT(vec1 == Vector4(1, 3, 2, 1.5));
    vec1 *= Vector4(2, 1, 2, 4);
    TEST_ASSERT(vec1 == vec2);

    return TEST_SUCCESS;
}

TEST_FUNC(dot_product) {
    const Vector2 left2  = Vector2(2.5, 3);
    const Vector2 right2 = Vector2(-1, -5);
    TEST_ASSERT(vector::dot(left2, right2) == -17.5);

    const Vector3 left3  = Vector3(2.5, 3, 1);
    const Vector3 right3 = Vector3(-1, -5, 10);
    TEST_ASSERT(vector::dot(left3, right3) == -7.5);

    const Vector4 left4  = Vector4(2.5, 3, 1, 2.5);
    const Vector4 right4 = Vector4(-1, -5, 10, -0.5);
    TEST_ASSERT(vector::dot(left4, right4) == -8.75);

    return TEST_SUCCESS;
}

TEST_FUNC(cross_product) {
    const Vector3 left  = Vector3(3, -3, 1);
    const Vector3 right = Vector3(4, 9, 2);
    TEST_ASSERT(vector::cross(left, right) == Vector3(-15, -2, 39))
    return TEST_SUCCESS;
}
