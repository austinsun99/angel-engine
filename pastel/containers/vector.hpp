#pragma once

#include "core/logging/logger.h"
#include <cstring>
#include "pastel_types.h"
#include "core/memory/memory.h"

namespace Pastel {
template <class T>
class Vector {
   private:
    Memory::MemoryCategory _memtype;
    u32 _capacity;
    u32 _count;
    T *_ptr;

    void resize(u32 count);

   public:
    Vector(Memory::MemoryCategory memtype);
    Vector();
    ~Vector();
    T &operator[](u32 index);

    void push_back(T item);
    T &pop_back();
    T &get(u32 index);

    u32 count();
};

template <class T>
Vector<T>::Vector() {
    _memtype  = Memory::MemoryCategory::MEMORY_CATEGORY_UNKNOWN;
    _count    = 0;
    _ptr      = pt_memnew_many(_memtype, T, 8);
    _capacity = 8;
}

template <class T>
Vector<T>::Vector(Memory::MemoryCategory memtype) {
    _memtype  = memtype;
    _count    = 0;
    _ptr      = pt_memnew_many(_memtype, T, 8);
    _capacity = 8;
}

template <class T>
Vector<T>::~Vector() {
    pt_memdelete_many(_memtype, _ptr, _capacity);
}

template <class T>
T &Vector<T>::operator[](u32 index) {
    return _ptr[index];
}

template <class T>
void Vector<T>::push_back(T item) {
    if (_capacity == 0) {
        resize(8);
    }
    if (_count == _capacity) {
        // @todo: better capacity algorithm
        resize(_capacity * 2);
    }

    _ptr[_count] = item;
    ++_count;
}

template <class T>
T &Vector<T>::pop_back() {
    if (_count == 0) {
        CORE_LOG_ERROR("Attempting to pop a vector with no elements")
    }

    --_count;
    return _ptr[_count];
}

template <class T>
void Vector<T>::resize(u32 new_capacity) {
    if (new_capacity <= _capacity) return;
    T *temp = pt_memnew_many(_memtype, T, new_capacity);
    memcpy(temp, _ptr, sizeof(T) * _count);
    pt_memdelete_many(_memtype, _ptr, _capacity);
    _ptr      = temp;
    _capacity = new_capacity;
}

template <class T>
T &Vector<T>::get(u32 index) {
    if (index >= _count) {
        CORE_LOG_ERROR("Attempting to access an out of bounds index %d, but size is %d", index, _count)
    }
    return this[index];
}

template <class T>
u32 Vector<T>::count() {
    return _count;
}

}  // namespace Pastel
