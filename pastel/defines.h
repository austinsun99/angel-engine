#pragma once

/**
 * PLATFORM DETECTION
 */
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#    ifndef _WIN64
#        error "64-bit is required"
#    endif
#    define PLATFORM_WINDOWS
#endif

constexpr bool platform_windows =
#ifdef PLATFORM_WINDOWS
    true;
#else
    false;
#endif

#if defined(__gnu_linux__) || defined(__linux__)
#    define PLATFORM_LINUX
#    if defined(__ANDROID__) || defined(__ANDROID_API__)
#        define PLATFORM_ANDROID
#    endif
#endif

constexpr bool platform_linux =
#ifdef PLATFORM_LINUX
    true;
#else
    false;
#endif

#if defined(__unix__) || defined(__unix)
#    define PLATFORM_UNIX
#endif

#if defined(macintosh) || defined(Macintosh) || defined(__APPLE__)
#    define PLATFORM_MACOS
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_MACOS)
#    error "Platform not supported"
#endif

/**
 * EXPORTS
 */
#ifdef PASTEL_EXPORT
#    ifdef PLATFORM_WINDOWS
#        define PASTEL_API __declspec(dllexport)
#    else
#        define PASTEL_API __attribute__((visibility("default")))
#    endif
#else
#    ifdef PLATFORM_WINDOWS
#        define PASTEL_API __declspec(dllimport)
#    else
#        define PASTEL_API
#    endif
#endif
