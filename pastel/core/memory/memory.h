#pragma once

#include <cstddef>
#include <utility>
#include "defines.h"

namespace Pastel::Memory {

enum MemoryCategory {
    MEMORY_CATEGORY_UNKNOWN,
    MEMORY_CATEGORY_PLATFORM,
    MEMORY_CATEGORY_MAX_CATEGORIES,
};

void init_memory();
void deinit_memory();
void record_mem_alloc(MemoryCategory memtype, std::size_t size);
void record_mem_dealloc(MemoryCategory memtype, std::size_t size);
void log_mem_alloc_info();

template <class T, class... Args>
PASTEL_FORCE_INLINE static T *_memnew(MemoryCategory memtype, Args &&...args) {
    record_mem_alloc(memtype, sizeof(T));
    return new T(std::forward<Args>(args)...);
}

template <class T>
PASTEL_FORCE_INLINE static void _memdelete(MemoryCategory memtype, T *ptr) {
    record_mem_dealloc(memtype, sizeof(T));
    delete (ptr);
}

#define pt_memnew(category, type, ...) ::Pastel::Memory::_memnew<type>(category, ##__VA_ARGS__)
#define pt_memdelete(category, ptr) ::Pastel::Memory::_memdelete(category, ptr)

}  // namespace Pastel::Memory
