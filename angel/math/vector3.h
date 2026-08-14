#pragma once

namespace angel::math {
struct Vector3 {
    union {
        struct {
            float x, y, z;
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

    constexpr bool operator==(const Vector3 &right) const {
        return this->x == right.x && this->y == right.y && this->z == right.z;
    }

    constexpr Vector3 operator+(const Vector3 &right) const {
        return Vector3(this->x + right.x, this->y + right.y, this->z + right.z);
    };

    constexpr Vector3 &operator+=(const Vector3 &right) {
        this->x += right.x;
        this->y += right.y;
        this->z += right.z;
        return *this;
    }

    constexpr Vector3 operator-(const Vector3 &right) const {
        return Vector3(this->x - right.x, this->y - right.y, this->z - right.z);
    };

    constexpr Vector3 &operator-=(const Vector3 &right) {
        this->x -= right.x;
        this->y -= right.y;
        this->z -= right.z;
        return *this;
    }

    constexpr Vector3 operator*(const Vector3 &right) const {
        return Vector3(this->x * right.x, this->y * right.y, this->z * right.z);
    }

    constexpr Vector3 operator*(const float fac) const {
        return Vector3(this->x * fac, this->y * fac, this->z * fac);
    }

    constexpr Vector3 &operator*=(const Vector3 &right) {
        this->x *= right.x;
        this->y *= right.y;
        this->z *= right.z;
        return *this;
    }

    constexpr Vector3 &operator*=(const float &fac) {
        this->x *= fac;
        this->y *= fac;
        this->z *= fac;
        return *this;
    }

    constexpr Vector3 operator/(const Vector3 &right) const {
        return Vector3(this->x / right.x, this->y / right.y, this->z / right.z);
    }

    constexpr Vector3 &operator/=(const Vector3 &right) {
        this->x /= right.x;
        this->y /= right.y;
        this->z /= right.z;
        return *this;
    }

    constexpr float dot(const Vector3 &right) const {
        return this->x * right.x + this->y * right.y + this->z * right.z;
    }
};

}  // namespace angel::math
