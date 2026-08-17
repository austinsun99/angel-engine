#pragma once

#include <cstddef>
#include "angel_types.h"
#include "core/logging/asserts.h"
#include "core/logging/logger.h"
#include "defines.h"
namespace angel::math {

template <int r, int c>
class Matrixf {
   private:
    float _elems[r * c];
    const u64 _size;

   public:
    Matrixf() : _size(r * c) {
        for (u64 i = 0; i < _size; ++i) {
            _elems[i] = 0;
        }
    }
    Matrixf(const float (&elems)[r * c]) : _size(r * c) {
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

    // 0 indexed
    ANGEL_FORCE_INLINE constexpr bool is_valid_index(u64 row, u64 col) const {
        return row < r && col < c;
    }

    ANGEL_FORCE_INLINE constexpr bool operator==(const Matrixf<r, c> &other) const {
        for (u64 i = 0; i < r * c; ++i)
            if (_elems[i] != other._elems[i]) return false;
        return true;
    }

    ANGEL_FORCE_INLINE constexpr Matrixf<r, c> operator+(const Matrixf<r, c> &other) {
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

    ANGEL_FORCE_INLINE constexpr Matrixf<r, c> operator-(const Matrixf<r, c> &other) {
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

    ANGEL_FORCE_INLINE constexpr void print() {
        AL_CORE_DEBUG("size (%d):", _size);
        for (u64 i = 0; i < _size; ++i) {
            AL_CORE_DEBUG("(%d): %f", i, _elems[i]);
        }
    }
};

namespace matrix {
template <int r, int c1, int c2>
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
