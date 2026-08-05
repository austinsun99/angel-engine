#pragma once

#include "core/platform/platform.h"
#include "defines.h"

// @todo: add logging steps
namespace Pastel::Logger {

enum LogLevel {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
};

void logger_init();
void log_output(LogLevel level, const char *fmt, const char *file, int line, ...);

#define LOG_FATAL_ENABLED 1
#define LOG_ERROR_ENABLED 1
#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef PASTEL_DEBUG
#    define LOG_DEBUG_ENABLED 1
#    define LOG_TRACE_ENABLED 1
#elif defined(PASTEL_RELEASE)
#    define LOG_DEBUG_ENABLED 0
#    define LOG_TRACE_ENABLED 0
#endif

#if LOG_FATAL_ENABLED == 1
#    define CORE_LOG_FATAL(fmt, ...) log_output(Pastel::Logger::LOG_LEVEL_FATAL, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define CORE_LOG_FATAL(fmt, ...)
#endif

#if LOG_ERROR_ENABLED == 1
#    define CORE_LOG_ERROR(fmt, ...) log_output(Pastel::Logger::LOG_LEVEL_ERROR, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define CORE_LOG_ERROR(fmt, ...)
#endif

#if LOG_WARN_ENABLED == 1
#    define CORE_LOG_WARN(fmt, ...) log_output(Pastel::Logger::LOG_LEVEL_WARNING, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define CORE_LOG_WARN(fmt, ...)
#endif

#if LOG_INFO_ENABLED == 1
#    define CORE_LOG_INFO(fmt, ...) log_output(Pastel::Logger::LOG_LEVEL_INFO, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define CORE_LOG_INFO(fmt, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#    define CORE_LOG_DEBUG(fmt, ...) log_output(Pastel::Logger::LOG_LEVEL_DEBUG, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define CORE_LOG_DEBUG(fmt, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#    define CORE_LOG_TRACE(fmt, ...) log_output(Pastel::Logger::LOG_LEVEL_TRACE, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define CORE_LOG_TRACE(fmt, ...)
#endif

}  // namespace Pastel::Logger
