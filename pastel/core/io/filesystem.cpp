#include "filesystem.h"
#include "core/logging/logger.h"
#include <fstream>

namespace Pastel::Io {
std::vector<char> read_file(const char *file_name) {
    std::ifstream file(file_name, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        CORE_LOG_ERROR("(IO) Failed to open file %s. Returning an empty buffer.", file_name)
        return std::vector<char>();
    }

    std::vector<char> buf(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&buf[0], static_cast<std::streamsize>(buf.size()));
    file.close();
    return buf;
}
}  // namespace Pastel::Io
