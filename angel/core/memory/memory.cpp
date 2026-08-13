#include "memory.h"
#include "core/logging/logger.h"
#include "angel_types.h"
#include "core/logging/asserts.h"

#include <cstdio>
#include <cstring>
#include <inttypes.h>

namespace angel::memory {

#if defined(ANGEL_DEBUG)
static u64 memory_allocations[MEMORY_CATEGORY_MAX_CATEGORIES];
constexpr const char *mem_category_names[MEMORY_CATEGORY_MAX_CATEGORIES] = {
    "Unknown",
    "Platform",
};
#endif

void init_memory() {
#if defined(ANGEL_DEBUG)
    memset(memory_allocations, 0, sizeof(memory_allocations));
#endif
}

void deinit_memory() {
#if defined(ANGEL_DEBUG)
    for (int i = 0; i < MEMORY_CATEGORY_MAX_CATEGORIES; ++i) {
        if (memory_allocations[i] > 0) {
            AL_CORE_WARN("Memory leak detected! See more information below.")
        }
    }
    log_mem_alloc_info();
#endif
}

void record_mem_alloc(MemoryCategory memtype, std::size_t size) {
#if defined(ANGEL_DEBUG)
    if (memtype == MEMORY_CATEGORY_UNKNOWN) {
        AL_CORE_WARN(
            "Allocating %d bytes of memory with category: unknown. Consider categorizing the memory allocation.",
            size)
    }
    memory_allocations[memtype] += size;
#else
    (void)memtype;
    (void)size;
#endif
}

void record_mem_dealloc(MemoryCategory memtype, std::size_t size) {
#if defined(ANGEL_DEBUG)
    if (memtype == MEMORY_CATEGORY_UNKNOWN) {
        AL_CORE_WARN(
            "Deallocating %d bytes of memory with category: unknown. Consider categorizing the memory allocation.",
            size)
    }
    memory_allocations[memtype] -= size;
#else
    (void)memtype;
    (void)size;
#endif
}

void log_mem_alloc_info() {
#if defined(ANGEL_DEBUG)
    char buf[8192];
    memset(buf, 0, sizeof(buf));

    ANGEL_ASSERT(strlen(buf) == 0);
    u64 offset = 0;
    for (int i = 0; i < MEMORY_CATEGORY_MAX_CATEGORIES; ++i) {
        int bytes_written = snprintf(buf + offset,
                                     sizeof(buf) - offset,
                                     "%s: %" PRIu64 " bytes allocated\n",
                                     mem_category_names[i],
                                     memory_allocations[i]);
        offset += bytes_written;
    }
    AL_CORE_INFO("\n---------------\nMemory Info:\n %s---------------", buf)
#else
    AL_CORE_INFO("Memory info is only available in debug mode.")
#endif
}

}  // namespace angel::memory
