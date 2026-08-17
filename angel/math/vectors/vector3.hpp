#pragma once

#include "defines.h"

namespace angel::math {
struct Vector3 {
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
        };
        float values[3];
    };

    constexpr Vector3() : x(0), y(0), z(0) {};
    constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
    constexpr Vector3(const Vector3 &vec) = default;
    ~Vector3()                            = default;

    Vector3(Vector3 &&vec)                 = delete;
    Vector3 &operator=(const Vector3 &vec) = default;
    Vector3 &operator=(Vector3 &&vec)      = delete;

    ANGEL_FORCE_INLINE constexpr const float &operator[](int i) const {
        return values[i];
    }

    ANGEL_FORCE_INLINE constexpr bool operator==(const Vector3 &right) const {
        return this->x == right.x && this->y == right.y && this->z == right.z;
    }

    ANGEL_FORCE_INLINE constexpr Vector3 operator+(const Vector3 &right) const {
        return Vector3(this->x + right.x, this->y + right.y, this->z + right.z);
    };

    ANGEL_FORCE_INLINE constexpr Vector3 &operator+=(const Vector3 &right) {
        this->x += right.x;
        this->y += right.y;
        this->z += right.z;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector3 operator-(const Vector3 &right) const {
        return Vector3(this->x - right.x, this->y - right.y, this->z - right.z);
    };

    ANGEL_FORCE_INLINE constexpr Vector3 &operator-=(const Vector3 &right) {
        this->x -= right.x;
        this->y -= right.y;
        this->z -= right.z;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector3 operator*(const Vector3 &right) const {
        return Vector3(this->x * right.x, this->y * right.y, this->z * right.z);
    }

    ANGEL_FORCE_INLINE constexpr Vector3 operator*(const float fac) const {
        return Vector3(this->x * fac, this->y * fac, this->z * fac);
    }

    ANGEL_FORCE_INLINE constexpr Vector3 &operator*=(const Vector3 &right) {
        this->x *= right.x;
        this->y *= right.y;
        this->z *= right.z;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector3 &operator*=(const float &fac) {
        this->x *= fac;
        this->y *= fac;
        this->z *= fac;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector3 operator/(const Vector3 &right) const {
        return Vector3(this->x / right.x, this->y / right.y, this->z / right.z);
    }

    ANGEL_FORCE_INLINE constexpr Vector3 operator/(const float right) const {
        return Vector3(this->x / right, this->y / right, this->z / right);
    }

    ANGEL_FORCE_INLINE constexpr Vector3 operator/(const double right) const {
        return Vector3(this->x / right, this->y / right, this->z / right);
    }

    ANGEL_FORCE_INLINE constexpr Vector3 &operator/=(const Vector3 &right) {
        this->x /= right.x;
        this->y /= right.y;
        this->z /= right.z;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector3 &operator/=(const float right) {
        this->x /= right;
        this->y /= right;
        this->z /= right;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector3 &operator/=(const double right) {
        this->x /= right;
        this->y /= right;
        this->z /= right;
        return *this;
    }
};  // namespace angel::math
}  // namespace angel::math
