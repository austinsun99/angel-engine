#pragma once

#include "core/application.h"
#include "core/logging/logger.h"

namespace angel {

struct EngineStartInfo {
    int argc;
    char **argv;
};

extern bool create_application(angel::EngineStartInfo start_info, ::angel::ApplicationCreateInfo *out_config);

}  // namespace angel

int main(int argc, char **argv) {
    angel::EngineStartInfo start_info{
        .argc = argc,
        .argv = argv,
    };

    angel::ApplicationCreateInfo create_info;
    create_application(start_info, &create_info);

    if (create_info.init == nullptr || create_info.update == nullptr || create_info.close == nullptr) {
        AL_CORE_FATAL("Function pointers not found for application. Exiting...")
        return -1;
    }

    angel::Application app = angel::Application();
    app.create(create_info);

    create_info.init();
    while (app.app_update()) {
        create_info.update(app.delta_time());
    };
    create_info.close();

    app.destroy();
    return 0;
}
