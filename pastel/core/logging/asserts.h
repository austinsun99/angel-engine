#pragma once

#include "defines.h"
#include "assert.h"

namespace Pastel {
bool log_assert_fail(const char *expr, const char *msg, const char *file, int line);

#if _MSC_VER
#    include <intrin.h>
#    define HALT() __debugbreak()
#else
#    define HALT() __builtin_trap()
#endif

#ifdef PASTEL_DEBUG
#    define PASTEL_ASSERT(expr) (!(expr) && ::Pastel::log_assert_fail(#expr, "", __FILE__, __LINE__) && (HALT(), 1));
#    define PASTEL_ASSERT_MSG(expr, msg) (!(expr) && log_assert_fail(#expr, msg, __FILE__, __LINE__) && (HALT(), 1));
#else
#    define PASTEL_ASSERT(expr)
#    define PASTEL_ASSERT_MSG(expr, msg)
#endif

}  // namespace Pastel
