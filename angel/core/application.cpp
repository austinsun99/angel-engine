#include "application.h"
#include "core/logging/logger.h"
#include "core/memory/memory.h"
#include "core/platform/platform.h"
#include "renderer/vulkan_renderer.h"

namespace angel {
bool Application::create(ApplicationCreateInfo create_info) {
    platform::WindowConfig config = {
        .x                = 0,
        .y                = 0,
        .width            = 1920,
        .height           = 1080,
        .application_name = create_info.application_name,
    };

    memory::init_memory();
    _window = platform::Window();

    logger::logger_init();

    if (!_window.open_window(config)) {
        AL_CORE_FATAL("Failed to open window")
        return false;
    }
    platform::clear_terminal_colour();

    _renderer.init(_window, nullptr);
    return true;
}

bool Application::app_update() {
    _window.update();
    if (!_window.pump_window()) {
        _window.running = false;
        return false;
    }
    _renderer.update_start();

    const double time = platform::get_time();
    _delta_time       = time - _prev_time;
    _prev_time        = time;
    return _window.running;
}

void Application::destroy() {
    _window.deinit();
    memory::deinit_memory();
}

}  // namespace angel
