// Copyright © 2026 Caleb Cardin. All rights reserved.
module;


export module TypeDefs;
import std;

export namespace Corvorium {
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;


namespace Detail {
template<typename T>
concept Boolean = std::is_convertible_v<T, bool>;

using SourceLocation = std::source_location;
}
}
