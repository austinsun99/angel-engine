#pragma once

#include "defines.h"

namespace angel::math {
struct Vector4 {
    union {
        struct {
            union {
                float x;
                float r;
            };
            union {
                float y;
                float g;
            };
            union {
                float z;
                float b;
            };
            union {
                float w;
                float a;
            };
        };
        float values[4];
    };

    constexpr Vector4() : x(0), y(0), z(0), w(0) {};
    constexpr Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};
    constexpr Vector4(const Vector4 &vec) = default;
    ~Vector4()                            = default;

    Vector4(Vector4 &&vec)                 = delete;
    Vector4 &operator=(const Vector4 &vec) = default;
    Vector4 &operator=(Vector4 &&vec)      = delete;

    ANGEL_FORCE_INLINE constexpr const float &operator[](int i) const {
        return values[i];
    }

    ANGEL_FORCE_INLINE constexpr bool operator==(const Vector4 &right) const {
        return this->x == right.x && this->y == right.y && this->z == right.z && this->w == right.w;
    }

    ANGEL_FORCE_INLINE constexpr Vector4 operator+(const Vector4 &right) const {
        return Vector4(this->x + right.x, this->y + right.y, this->z + right.z, this->w + right.w);
    };

    ANGEL_FORCE_INLINE constexpr Vector4 &operator+=(const Vector4 &right) {
        this->x += right.x;
        this->y += right.y;
        this->z += right.z;
        this->w += right.w;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector4 operator-(const Vector4 &right) const {
        return Vector4(this->x - right.x, this->y - right.y, this->z - right.z, this->w - right.w);
    };

    ANGEL_FORCE_INLINE constexpr Vector4 &operator-=(const Vector4 &right) {
        this->x -= right.x;
        this->y -= right.y;
        this->z -= right.z;
        this->w -= right.w;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector4 operator*(const Vector4 &right) const {
        return Vector4(this->x * right.x, this->y * right.y, this->z * right.z, this->w * right.w);
    }

    ANGEL_FORCE_INLINE constexpr Vector4 operator*(const float fac) const {
        return Vector4(this->x * fac, this->y * fac, this->z * fac, this->w * fac);
    }

    ANGEL_FORCE_INLINE constexpr Vector4 &operator*=(const Vector4 &right) {
        this->x *= right.x;
        this->y *= right.y;
        this->z *= right.z;
        this->w *= right.w;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector4 &operator*=(const float &fac) {
        this->x *= fac;
        this->y *= fac;
        this->z *= fac;
        this->w *= fac;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector4 operator/(const Vector4 &right) const {
        return Vector4(this->x / right.x, this->y / right.y, this->z / right.z, this->w / right.w);
    }

    ANGEL_FORCE_INLINE constexpr Vector4 &operator/=(const Vector4 &right) {
        this->x /= right.x;
        this->y /= right.y;
        this->z /= right.z;
        this->w /= right.w;
        return *this;
    }
};
}  // namespace angel::math
