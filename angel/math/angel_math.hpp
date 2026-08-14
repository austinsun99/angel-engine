#pragma once

#include <cmath>
#include "angel_types.h"
#include "defines.h"
#include "core/logging/asserts.h"
namespace angel::math {

#define AL_PI 3.14159265358979323846264
#define AL_HALF_PI AL_PI / 2.0

#define AL_DEG_TO_RAD_FACTOR AL_PI / 180.0
#define AL_RAD_TO_DEG_FACTOR 180.0 / AL_PI

constexpr static double pi      = AL_PI;
constexpr static double half_pi = AL_HALF_PI;

ANGEL_FORCE_INLINE constexpr static u64 powf(float b, u64 n) {
    if (n == 0) return 1;
    while (n-- > 1) {
        b *= b;
    }
    return b;
}

ANGEL_FORCE_INLINE constexpr static u64 fac(u64 n) {
    if (n == 0 || n == 1) return 1;
    u64 result = n;
    while (n-- > 1) {
        result *= n;
    }
    return result;
};

ANGEL_FORCE_INLINE constexpr static float absf(float x) {
    return x < 0 ? -x : x;
};

ANGEL_FORCE_INLINE constexpr static double absf(double x) {
    return x < 0 ? -x : x;
};

//
// Trig
//

ANGEL_FORCE_INLINE constexpr static float deg_to_radf(float deg) {
    return deg * AL_DEG_TO_RAD_FACTOR;
}

ANGEL_FORCE_INLINE constexpr static float rad_to_degf(float rad) {
    return rad * AL_RAD_TO_DEG_FACTOR;
}

ANGEL_FORCE_INLINE constexpr static float sinf_rad(float rad) {
    return std::sinf(rad);
}

ANGEL_FORCE_INLINE constexpr static float sinf_deg(float deg) {
    return std::sinf(deg_to_radf(deg));
}

ANGEL_FORCE_INLINE constexpr static float cosf_rad(float rad) {
    return std::cosf(rad);
}

ANGEL_FORCE_INLINE constexpr static float cosf_deg(float deg) {
    return std::cosf(deg_to_radf(deg));
}

ANGEL_FORCE_INLINE constexpr static float tanf_rad(float rad) {
    return std::tanf(rad);
}

ANGEL_FORCE_INLINE constexpr static float tanf_deg(float deg) {
    return std::tanf(deg_to_radf(deg));
}

ANGEL_FORCE_INLINE constexpr static float asinf(float val) {
    ANGEL_ASSERT(val >= -1 && val <= 1)
    return std::asinf(val);
}

ANGEL_FORCE_INLINE constexpr static float acosf(float val) {
    ANGEL_ASSERT(val >= -1 && val <= 1)
    return std::acosf(val);
}

ANGEL_FORCE_INLINE constexpr static float atanf(float val) {
    ANGEL_ASSERT(val >= -1 && val <= 1)
    return std::atanf(val);
}

}  // namespace angel::math
