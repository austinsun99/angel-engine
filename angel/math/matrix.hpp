#pragma once

#include "angel_types.h"
#include "core/logging/asserts.h"
#include "defines.h"
namespace angel::math {

template <u64 r, u64 c>
class Matrixf {
   private:
    float _elems[r * c];
    static constexpr u64 _size = r * c;

   public:
    Matrixf() {
        for (u64 i = 0; i < _size; ++i) {
            _elems[i] = 0;
        }
    }
    Matrixf(const float (&elems)[r * c]) {
        for (u64 i = 0; i < _size; ++i) {
            _elems[i] = elems[i];
        }
    }

    ~Matrixf() = default;

    // 0 indexed
    ANGEL_FORCE_INLINE constexpr float indexed(u64 row, u64 col) const {
        ANGEL_ASSERT(is_valid_index(row, col))
        return _elems[row * c + col];
    }

    // Does not do any bounds checking. 0 indexed
    ANGEL_FORCE_INLINE constexpr const float *operator[](u64 row) const {
        return _elems + row * c;
    }

    // 0 indexed
    ANGEL_FORCE_INLINE constexpr bool is_valid_index(u64 row, u64 col) const {
        return row < r && col < c;
    }

    ANGEL_FORCE_INLINE constexpr bool operator==(const Matrixf<r, c> &other) const {
        for (u64 i = 0; i < r * c; ++i)
            if (_elems[i] != other._elems[i]) return false;
        return true;
    }

    ANGEL_FORCE_INLINE constexpr const Matrixf<r, c> operator+(const Matrixf<r, c> &other) const {
        float temp[_size];
        for (u64 i = 0; i < _size; ++i) {
            temp[i] = _elems[i] + other._elems[i];
        }
        return Matrixf(temp);
    }

    ANGEL_FORCE_INLINE constexpr Matrixf<r, c> &operator+=(const Matrixf<r, c> &other) {
        for (u64 i = 0; i < _size; ++i) {
            this->_elems[i] += other._elems[i];
        }
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr const Matrixf<r, c> operator-(const Matrixf<r, c> &other) const {
        float temp[_size];
        for (u64 i = 0; i < _size; ++i) {
            temp[i] = _elems[i] - other._elems[i];
        }
        return Matrixf(temp);
    }

    ANGEL_FORCE_INLINE constexpr Matrixf<r, c> &operator-=(const Matrixf<r, c> &other) {
        for (u64 i = 0; i < _size; ++i) {
            this->_elems[i] -= other._elems[i];
        }
        return *this;
    }

    ANGEL_FORCE_INLINE constexpr u64 rows_count() {
        return r;
    }

    ANGEL_FORCE_INLINE constexpr u64 cols_count() {
        return c;
    }
};

typedef Matrixf<4, 4> Matrix4x4;
typedef Matrixf<3, 3> Matrix3x3;
typedef Matrixf<2, 2> Matrix2x2;

namespace matrix {
template <u64 r, u64 c1, u64 c2>
ANGEL_FORCE_INLINE static constexpr Matrixf<r, c2> mult(const Matrixf<r, c1> &left, const Matrixf<c1, c2> &right) {
    float elems[r * c2] = {};
    for (u64 row = 0; row < r; ++row) {
        for (u64 col = 0; col < c2; ++col) {
            float sum = 0.f;
            for (u64 i = 0; i < c1; ++i) {
                sum += left.indexed(row, i) * right.indexed(i, col);
            }
            elems[row * c2 + col] = sum;
        }
    }
    return Matrixf<r, c2>(elems);
}
}  // namespace matrix

}  // namespace angel::math
