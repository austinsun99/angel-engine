#pragma once

#include "defines.h"

namespace angel::math {
struct Vector2 {
    union {
        struct {
            float values[2];
            struct {
                float x;
                float y;
            };
        };
    };

    constexpr Vector2() : x(0), y(0) {};
    constexpr Vector2(float x, float y) : x(x), y(y) {};
    constexpr Vector2(const Vector2 &vec) = default;
    ~Vector2()                            = default;

    Vector2(Vector2 &&vec)                 = delete;
    Vector2 &operator=(const Vector2 &vec) = default;
    Vector2 &operator=(Vector2 &&vec)      = delete;

    ANGEL_FORCE_INLINE constexpr const float &operator[](int i) const {
        return values[i];
    }

    ANGEL_FORCE_INLINE constexpr bool operator==(const Vector2 &right) const {
        return this->x == right.x && this->y == right.y;
    }

    ANGEL_FORCE_INLINE constexpr Vector2 operator+(const Vector2 &right) const {
        return Vector2(this->x + right.x, this->y + right.y);
    };

    ANGEL_FORCE_INLINE constexpr Vector2 &operator+=(const Vector2 &right) {
        this->x += right.x;
        this->y += right.y;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector2 operator-(const Vector2 &right) const {
        return Vector2(this->x - right.x, this->y - right.y);
    };

    ANGEL_FORCE_INLINE constexpr Vector2 &operator-=(const Vector2 &right) {
        this->x -= right.x;
        this->y -= right.y;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector2 operator*(const Vector2 &right) const {
        return Vector2(this->x * right.x, this->y * right.y);
    }

    ANGEL_FORCE_INLINE constexpr Vector2 operator*(const float fac) const {
        return Vector2(this->x * fac, this->y * fac);
    }

    ANGEL_FORCE_INLINE constexpr Vector2 &operator*=(const Vector2 &right) {
        this->x *= right.x;
        this->y *= right.y;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector2 &operator*=(const float &fac) {
        this->x *= fac;
        this->y *= fac;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector2 operator/(const Vector2 &right) const {
        return Vector2(this->x / right.x, this->y / right.y);
    }

    ANGEL_FORCE_INLINE constexpr Vector2 operator/(const float right) const {
        return Vector2(this->x / right, this->y / right);
    }

    ANGEL_FORCE_INLINE constexpr Vector2 operator/(const double right) const {
        return Vector2(this->x / right, this->y / right);
    }

    ANGEL_FORCE_INLINE constexpr Vector2 &operator/=(const Vector2 &right) {
        this->x /= right.x;
        this->y /= right.y;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector2 &operator/=(const float right) {
        this->x /= right;
        this->y /= right;
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr Vector2 &operator/=(const double right) {
        this->x /= right;
        this->y /= right;
        return *this;
    }
};

}  // namespace angel::math
