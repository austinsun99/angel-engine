#include <core/platform/input.h>
#include <core/platform/platform.h>
#include <core/application.h>
#include "core/entry_point.h"
#include "core/logging/logger.h"

#include "math/matrix.hpp"

using namespace angel;
using namespace angel::math;

bool init() {
    float elems1[]   = {0, 1, 2, 3, 4, 5};
    float elems2[]   = {2, 1, 4, 5, 1.5, 3, 2, 1, 1, 0, 0, 8};
    Matrixf<2, 3> m1 = Matrixf<2, 3>(elems1);
    Matrixf<3, 4> m2 = Matrixf<3, 4>(elems2);
    m1.print();
    m2.print();

    float result[]         = {3.5, 3, 2, 17, 17, 15, 20, 59};
    Matrixf<2, 4> mult_mat = Matrixf<2, 4>(result);

    Matrixf result_mat = matrix::mult(m1, m2);
    mult_mat.print();
    result_mat.print();
    return true;
}

void update(double dt) {
    (void)dt;
    // static int frame = 0;
    // double fps       = 1.f / dt;
    // AL_CORE_DEBUG("frame: %d, fps: %f", frame, fps);
    // ++frame;
}

bool close() {
    return true;
}

bool angel::create_application(angel::EngineStartInfo start_info, angel::ApplicationCreateInfo *out_info) {
    (void)start_info;
    *out_info = {
        .application_name = "Engine",
        .init             = init,
        .update           = update,
        .close            = close,
    };
    return true;
}
