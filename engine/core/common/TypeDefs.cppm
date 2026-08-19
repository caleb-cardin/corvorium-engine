// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include <cstdint>
#include <source_location>
#include <type_traits>

export module Common:TypeDefs;

export namespace Corvorium {
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;


namespace Detail {
template<typename T>
concept Boolean = std::is_convertible_v<T, bool>;

using SourceLocation = std::source_location;
}
}
