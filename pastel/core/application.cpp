#include "application.h"
#include "core/logging/logger.h"
#include "core/memory/memory.h"
#include "core/platform/platform.h"
#include "renderer/vulkan_renderer.h"

namespace Pastel {
bool Application::create(ApplicationCreateInfo create_info) {
    Platform::WindowConfig config = {
        .x                = 0,
        .y                = 0,
        .width            = 1920,
        .height           = 1080,
        .application_name = create_info.application_name,
    };

    Memory::init_memory();
    _window = Platform::Window();

    Logger::logger_init();

    if (!_window.open_window(config)) {
        CORE_LOG_FATAL("Failed to open window")
        return false;
    }
    Platform::clear_terminal_colour();

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

    const double time = Platform::get_time();
    _delta_time       = time - _prev_time;
    _prev_time        = time;
    return _window.running;
}

void Application::destroy() {
    _window.deinit();
    Memory::deinit_memory();
}

}  // namespace Pastel
