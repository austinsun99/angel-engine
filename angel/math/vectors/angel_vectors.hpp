#pragma once

#include "defines.h"
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
ANGEL_FORCE_INLINE float dot(const Vector2 &left, const Vector2 &right) {
    return left.x * right.x + left.y * right.y;
}
ANGEL_FORCE_INLINE float dot(const Vector3 &left, const Vector3 &right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}
ANGEL_FORCE_INLINE float dot(const Vector4 &left, const Vector4 &right) {
    return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
}

}  // namespace angel::math::vector
