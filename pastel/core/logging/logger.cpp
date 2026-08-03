#include "logger.h"
#include "asserts.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "core/io/terminal_colours.h"
#include "core/platform/platform.h"

static Pastel::WindowState *window_state;

namespace Pastel {
bool log_assert_fail(const char *expr, const char *msg, const char *file, int line) {
    const Io::TerminalColour fg = Io::TERMINAL_COLOUR_RED;

    char msg_buf[8192]       = {};
    char assertion_msg[4096] = {};
    if (strcmp(msg, "") != 0) {
        std::snprintf(assertion_msg, sizeof(assertion_msg), "\n# %s", msg);
    }

    std::snprintf(msg_buf,
                  sizeof(msg_buf),
                  "\n# Assertion %s failed \n# @ %s\n# @ line %d%s\n",
                  expr,
                  file,
                  line,
                  assertion_msg);
    window_state->print_terminal(msg_buf, fg);
    window_state->clear_terminal_colour();
    return true;
}
}  // namespace Pastel

namespace Pastel::Logger {

void logger_init(WindowState *const state) {
    if (!state->console_is_initialized()) return;
    window_state = state;
}

void log_output(LogLevel level, const char *fmt, const char *file, int line, ...) {
    (void)file;
    (void)line;
    Io::TerminalColour bg = level == LOG_LEVEL_FATAL ? Io::TERMINAL_COLOUR_RED : Io::TERMINAL_COLOUR_NONE;
    Io::TerminalColour fg;
    const char *level_prefix;

    switch (level) {
        case LOG_LEVEL_FATAL:
            fg           = Io::TERMINAL_COLOUR_BLACK;
            level_prefix = "FATAL";
            break;
        case LOG_LEVEL_ERROR:
            fg           = Io::TERMINAL_COLOUR_RED;
            level_prefix = "ERROR";
            break;
        case LOG_LEVEL_WARNING:
            fg           = Io::TERMINAL_COLOUR_YELLOW;
            level_prefix = "WARNING";
            break;
        case LOG_LEVEL_INFO:
            fg           = Io::TERMINAL_COLOUR_LIGHTBLUE;
            level_prefix = "INFO";
            break;
        case LOG_LEVEL_DEBUG:
            fg           = Io::TERMINAL_COLOUR_WHITE;
            level_prefix = "DEBUG";
            break;
        case LOG_LEVEL_TRACE:
            fg           = Io::TERMINAL_COLOUR_GRAY;
            level_prefix = "TRACE";
            break;
    }

    char msg_buf[8192] = {};
    std::snprintf(msg_buf, sizeof(msg_buf), "[%s] %s\n", level_prefix, fmt);

    va_list arg_ptr;
    va_start(arg_ptr, line);
    char msg_buf2[8192] = {};
    std::vsnprintf(msg_buf2, sizeof(msg_buf2), msg_buf, arg_ptr);
    va_end(arg_ptr);

    window_state->print_terminal(msg_buf2, fg, bg);
}

}  // namespace Pastel::Logger
