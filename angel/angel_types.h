#pragma once

#include <cinttypes>
#include <cstdint>
#include "defines.h"

namespace angel {
typedef std::uint8_t u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

typedef std::int8_t i8;
typedef std::int16_t i16;
typedef std::int32_t i32;
typedef std::int64_t i64;

ANGEL_STATIC_ASSERT(sizeof(::angel::u8) == 1, "type u8 must be of size 1")
ANGEL_STATIC_ASSERT(sizeof(::angel::u16) == 2, "type u8 must be of size 2")
ANGEL_STATIC_ASSERT(sizeof(::angel::u32) == 4, "type u8 must be of size 4")
ANGEL_STATIC_ASSERT(sizeof(::angel::u64) == 8, "type u8 must be of size 8")

ANGEL_STATIC_ASSERT(sizeof(::angel::i8) == 1, "type i8 must be of size 1")
ANGEL_STATIC_ASSERT(sizeof(::angel::i16) == 2, "type i8 must be of size 2")
ANGEL_STATIC_ASSERT(sizeof(::angel::i32) == 4, "type i8 must be of size 4")
ANGEL_STATIC_ASSERT(sizeof(::angel::i64) == 8, "type i8 must be of size 8")
}  // namespace angel
