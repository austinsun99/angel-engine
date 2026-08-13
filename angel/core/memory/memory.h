#pragma once

#include <cstddef>
#include <utility>
#include "defines.h"
#include "angel_types.h"

namespace angel::memory {

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
ANGEL_FORCE_INLINE static T *_memnew(MemoryCategory memtype, Args &&...args) {
    record_mem_alloc(memtype, sizeof(T));
    return new T(std::forward<Args>(args)...);
}

template <class T>
ANGEL_FORCE_INLINE static void _memdelete(MemoryCategory memtype, T *ptr) {
    record_mem_dealloc(memtype, sizeof(T));
    delete (ptr);
}

template <class T>
ANGEL_FORCE_INLINE static T *_memnew_many(MemoryCategory memtype, u32 count) {
    record_mem_alloc(memtype, sizeof(T) * count);
    return new T[count];
}

template <class T>
ANGEL_FORCE_INLINE static void _memdelete_many(MemoryCategory memtype, T *ptr, u32 count) {
    record_mem_dealloc(memtype, sizeof(T) * count);
    delete[] ptr;
}

#define pt_memnew(category, type, ...) ::angel::memory::_memnew<type>(category, ##__VA_ARGS__)
#define pt_memdelete(category, ptr) ::angel::memory::_memdelete(category, ptr)
#define pt_memnew_many(category, type, count) ::angel::memory::_memnew_many<type>(category, count)
#define pt_memdelete_many(category, ptr, count) ::angel::memory::_memdelete_many(category, ptr, count)

}  // namespace angel::memory
