#pragma once

#include "defines.h"
#include "math/angel_math.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace angel::math::vector {

//
// Specific to a vector type
//

ANGEL_FORCE_INLINE Vector3 cross(const Vector3 &left, const Vector3 &right) {
    float x = left[1] * right[2] - left[2] * right[1];
    float y = -1 * (left[0] * right[2] - left[2] * right[0]);
    float z = left[0] * right[1] - left[1] * right[0];
    return Vector3(x, y, z);
}

//
// Generic for all vector types
//
ANGEL_FORCE_INLINE constexpr static float dot(const Vector2 &left, const Vector2 &right) {
    return left.x * right.x + left.y * right.y;
}

ANGEL_FORCE_INLINE constexpr static float dot(const Vector3 &left, const Vector3 &right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

ANGEL_FORCE_INLINE constexpr static float dot(const Vector4 &left, const Vector4 &right) {
    return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
}

ANGEL_FORCE_INLINE constexpr static float length_squared(const Vector2 &v) {
    return v.x * v.x + v.y * v.y;
}

ANGEL_FORCE_INLINE constexpr static float length_squared(const Vector3 &v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

ANGEL_FORCE_INLINE constexpr static float length_squared(const Vector4 &v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

ANGEL_FORCE_INLINE constexpr static float length(const Vector2 &v) {
    return math::sqrtf(length_squared(v));
}

ANGEL_FORCE_INLINE constexpr static float length(const Vector3 &v) {
    return math::sqrtf(length_squared(v));
}

ANGEL_FORCE_INLINE constexpr static float length(const Vector4 &v) {
    return math::sqrtf(length_squared(v));
}

ANGEL_FORCE_INLINE constexpr static Vector2 normalized(const Vector2 &v) {
    return v / length(v);
}

ANGEL_FORCE_INLINE constexpr static Vector3 normalized(const Vector3 &v) {
    return v / length(v);
}

ANGEL_FORCE_INLINE constexpr static Vector4 normalized(const Vector4 &v) {
    return v / length(v);
}

}  // namespace angel::math::vector
