#include <core/platform/input.h>
#include <core/platform/platform.h>
#include <core/application.h>
#include "core/entry_point.h"

using namespace angel;

bool init() {
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
