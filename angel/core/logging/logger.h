#pragma once

#include "core/platform/platform.h"
#include "defines.h"

// @todo: add logging steps
namespace angel::logger {

enum LogLevel {
    LOG_LEVEL_FATAL,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
};

void logger_init();
ANGEL_API void log_output(LogLevel level, const char *fmt, const char *file, int line, ...);

#define LOG_FATAL_ENABLED 1
#define LOG_ERROR_ENABLED 1
#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef ANGEL_DEBUG
#    define LOG_DEBUG_ENABLED 1
#    define LOG_TRACE_ENABLED 1
#elif defined(ANGEL_RELEASE)
#    define LOG_DEBUG_ENABLED 0
#    define LOG_TRACE_ENABLED 0
#endif

#if LOG_FATAL_ENABLED == 1
#    define AL_CORE_FATAL(fmt, ...) log_output(angel::logger::LOG_LEVEL_FATAL, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define AL_CORE_FATAL(fmt, ...)
#endif

#if LOG_ERROR_ENABLED == 1
#    define AL_CORE_ERROR(fmt, ...) log_output(angel::logger::LOG_LEVEL_ERROR, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define AL_CORE_ERROR(fmt, ...)
#endif

#if LOG_WARN_ENABLED == 1
#    define AL_CORE_WARN(fmt, ...) log_output(angel::logger::LOG_LEVEL_WARNING, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define AL_CORE_WARN(fmt, ...)
#endif

#if LOG_INFO_ENABLED == 1
#    define AL_CORE_INFO(fmt, ...) log_output(angel::logger::LOG_LEVEL_INFO, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define AL_CORE_INFO(fmt, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#    define AL_CORE_DEBUG(fmt, ...) log_output(angel::logger::LOG_LEVEL_DEBUG, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define AL_CORE_DEBUG(fmt, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#    define AL_CORE_TRACE(fmt, ...) log_output(angel::logger::LOG_LEVEL_TRACE, fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#    define AL_CORE_TRACE(fmt, ...)
#endif

}  // namespace angel::logger
