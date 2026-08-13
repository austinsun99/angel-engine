#pragma once

#include "core/application.h"
#include "core/logging/logger.h"

namespace Pastel {

struct EngineStartInfo {
    int argc;
    char **argv;
};

extern bool create_application(Pastel::EngineStartInfo start_info, ::Pastel::ApplicationCreateInfo *out_config);

}  // namespace Pastel

int main(int argc, char **argv) {
    Pastel::EngineStartInfo start_info{
        .argc = argc,
        .argv = argv,
    };

    Pastel::ApplicationCreateInfo create_info;
    create_application(start_info, &create_info);

    if (create_info.init == nullptr || create_info.update == nullptr || create_info.close == nullptr) {
        CORE_LOG_FATAL("Function pointers not found for application. Exiting...")
        return -1;
    }

    Pastel::Application app = Pastel::Application();
    app.create(create_info);

    create_info.init();
    while (app.app_update()) {
        create_info.update(app.delta_time());
    };
    create_info.close();

    app.destroy();
    return 0;
}
