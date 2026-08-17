#include "../angel_test.hpp"

#include "math/matrix.hpp"

using namespace angel::math;

TEST_FUNC(matrix_mult) {
    float elems1[]   = {0, 1, 2, 3, 4, 5};
    float elems2[]   = {2, 1, 4, 5, 1.5, 3, 2, 1, 1, 0, 0, 8};
    Matrixf<2, 3> m1 = Matrixf<2, 3>(elems1);
    Matrixf<3, 4> m2 = Matrixf<3, 4>(elems2);

    float result[]         = {3.5, 3, 2, 17, 17, 15, 20, 59};
    Matrixf<2, 4> mult_mat = Matrixf<2, 4>(result);

    Matrixf result_mat = matrix::mult(m1, m2);
    mult_mat.print();
    result_mat.print();

    TEST_ASSERT(result_mat == mult_mat);
    return TEST_SUCCESS;
}
