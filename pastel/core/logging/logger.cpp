#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include "core/io/terminal_colours.h"
#include "core/platform/platform.h"

namespace Pastel::Logger {

static WindowState *window_state;

void logger_init(WindowState *const state) {
    // @todo: check window state is initialized.
    window_state  = state;
}

void log_output(LogLevel level, const char *fmt, const char* file, int line, ...) {
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
