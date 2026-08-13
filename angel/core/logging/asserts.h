#pragma once

#include "defines.h"
#include "assert.h"

namespace angel {
bool log_assert_fail(const char *expr, const char *msg, const char *file, int line);

#if _MSC_VER
#    include <intrin.h>
#    define HALT() __debugbreak()
#else
#    define HALT() __builtin_trap()
#endif

#ifdef ANGEL_DEBUG
#    define ANGEL_ASSERT(expr) (!(expr) && ::angel::log_assert_fail(#expr, "", __FILE__, __LINE__) && (HALT(), 1));
#    define ANGEL_ASSERT_MSG(expr, msg) (!(expr) && log_assert_fail(#expr, msg, __FILE__, __LINE__) && (HALT(), 1));
#else
#    define ANGEL_ASSERT(expr)
#    define ANGEL_ASSERT_MSG(expr, msg)
#endif

}  // namespace angel
