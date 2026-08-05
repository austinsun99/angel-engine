#pragma once

#include <cstdint>
#include "defines.h"

namespace Pastel {
typedef std::uint8_t u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

typedef std::int8_t i8;
typedef std::int16_t i16;
typedef std::int32_t i32;
typedef std::int64_t i64;

PASTEL_STATIC_ASSERT(sizeof(::Pastel::u8) == 1, "type u8 must be of size 1")
PASTEL_STATIC_ASSERT(sizeof(::Pastel::u16) == 2, "type u8 must be of size 2")
PASTEL_STATIC_ASSERT(sizeof(::Pastel::u32) == 4, "type u8 must be of size 4")
PASTEL_STATIC_ASSERT(sizeof(::Pastel::u64) == 8, "type u8 must be of size 8")

PASTEL_STATIC_ASSERT(sizeof(::Pastel::i8) == 1, "type i8 must be of size 1")
PASTEL_STATIC_ASSERT(sizeof(::Pastel::i16) == 2, "type i8 must be of size 2")
PASTEL_STATIC_ASSERT(sizeof(::Pastel::i32) == 4, "type i8 must be of size 4")
PASTEL_STATIC_ASSERT(sizeof(::Pastel::i64) == 8, "type i8 must be of size 8")
}  // namespace Pastel
