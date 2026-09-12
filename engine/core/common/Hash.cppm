// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/string_view.h"
export module Hash;
import TypeDefs;
export namespace Corvorium {
namespace Hash {
inline constexpr u64 fnv1a_64(eastl::string_view Str)
{
    static constexpr u64 kOffsetBasis = 0xcbf2'9ce4'8422'2325;
    static constexpr u64 kPrime = 0x0000'0100'0000'01b3;
    u64 hash = kOffsetBasis;
    for (auto c: Str)
    {
        hash ^= c;
        hash *= kPrime;
    }
    return hash;
}
}

inline constexpr u64 operator""_hash(const char* Str, size_t Len)
{
    return Corvorium::Hash::fnv1a_64({Str});
}
}
