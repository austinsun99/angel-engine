#pragma once

#if defined(DEBUG)
#    define ANGEL_DEBUG 1
#elif defined(NDEBUG)
#    define ANGEL_RELEASE 1
#endif

#if defined(__clang__)
#    define ANGEL_COMPILER_CLANG 1
#elif defined(__GNUC__)
#    define ANGEL_COMPILER_GNU 1
#elif defined(_MSC_VER)
#    define ANGEL_COMPILER_MSVC 1
#endif

#ifndef ANGEL_FORCE_INLINE
#    if defined(ANGEL_COMPILER_GNU)
#        define ANGEL_FORCE_INLINE __attribute__((always_inline)) inline
#    elif defined(ANGEL_COMPILER_MSVC)
#        define ANGEL_FORCE_INLINE __forceinline
#    else
#        define ANGEL_FORCE_INLINE inline
#    endif
#endif

/**
 * PLATFORM DETECTION
 */
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
#    ifndef _WIN64
#        error "64-bit is required"
#    endif
#    define ANGEL_PLATFORM_WINDOWS
#endif

constexpr bool platform_windows =
#ifdef ANGEL_PLATFORM_WINDOWS
    true;
#else
    false;
#endif

#if defined(__gnu_linux__) || defined(__linux__)
#    define ANGEL_PLATFORM_LINUX
#    if defined(__ANDROID__) || defined(__ANDROID_API__)
#        define ANGEL_PLATFORM_ANDROID
#    endif
#endif

constexpr bool platform_linux =
#ifdef ANGEL_PLATFORM_LINUX
    true;
#else
    false;
#endif

#if defined(__unix__) || defined(__unix)
#    define ANGEL_PLATFORM_UNIX
#endif

#if defined(macintosh) || defined(Macintosh) || defined(__APPLE__)
#    define ANGEL_PLATFORM_MACOS
#endif

#if defined(ANGEL_PLATFORM_ANDROID) || defined(ANGEL_PLATFORM_MACOS)
#    error "Platform not supported"
#endif

#define ANGEL_STATIC_ASSERT(expr, msg) static_assert(expr, msg);

/**
 * EXPORTS
 */
#ifdef ANGEL_EXPORT
#    ifdef ANGEL_PLATFORM_WINDOWS
#        define ANGEL_API __declspec(dllexport)
#    else
#        define ANGEL_API __attribute__((visibility("default")))
#    endif
#else
#    ifdef ANGEL_PLATFORM_WINDOWS
#        define ANGEL_API __declspec(dllimport)
#    else
#        define ANGEL_API
#    endif
#endif
