#include <core/platform/input.h>
#include <core/platform/platform.h>
#include <core/application.h>
#include "core/entry_point.h"
#include "core/logging/logger.h"

bool init() {
    return true;
}

void update(double dt) {
    static int frame = 0;
    double fps = 1.f / dt;
    CORE_LOG_DEBUG("frame: %d, fps: %f", frame, fps);
    ++frame;
}

bool close() {
    return true;
}

bool Pastel::create_application(Pastel::EngineStartInfo start_info, Pastel::ApplicationCreateInfo *out_info) {
    (void)start_info;
    *out_info = {
        .application_name = "Engine",
        .init             = init,
        .update           = update,
        .close            = close,
    };
    return true;
}
