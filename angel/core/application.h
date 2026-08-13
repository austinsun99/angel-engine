#pragma once

#include "core/platform/platform.h"
#include "defines.h"
#include "renderer/vulkan_renderer.h"

namespace angel {

struct ANGEL_API ApplicationCreateInfo {
    const char *application_name;

    bool (*init)()                    = nullptr;
    void (*update)(double delta_time) = nullptr;
    bool (*close)()                   = nullptr;
};

class ANGEL_API Application {
   private:
    double _delta_time;
    double _prev_time;
    platform::Window _window;
    renderer::vulkan::VulkanRenderer _renderer;

   public:
    Application()  = default;
    ~Application() = default;

    bool create(ApplicationCreateInfo create_info);
    bool app_update();
    void destroy();

    double delta_time() const {
        return _delta_time;
    };
};

}  // namespace angel
