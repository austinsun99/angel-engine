#pragma once

#include "angel_types.h"
#include "core/logging/asserts.h"
#include "defines.h"
namespace angel::math {

template <int r, int c>
class Matrixf {
   private:
    float _elems[r * c];
    const int _size = r * c;

   public:
    Matrixf() {
        for (u64 i = 0; i < _size; ++i) {
            _elems[i] = 0;
        }
    }
    Matrixf(float elems[r * c]) {
        for (u64 i = 0; i < _size; ++i) {
            _elems[i] = elems[i];
        }
    }

    ~Matrixf() = default;

    // 1 indexed
    ANGEL_FORCE_INLINE constexpr float at(int row, int column) {
        return _elems[++row * r + ++column];
    }

    // 0 indexed
    ANGEL_FORCE_INLINE constexpr float indexed(int x, int y) {
        return _elems[x * r + y];
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
};

template <int r, int c1, int c2>
ANGEL_FORCE_INLINE static constexpr Matrixf<r, c2> matrix_mult(const Matrixf<r, c1> &left,
                                                               const Matrixf<c1, c2> &right) {
    // @todo
    (void)left;
    (void)right;
}

}  // namespace angel::math
