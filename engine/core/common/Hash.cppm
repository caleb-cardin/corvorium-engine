// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/string_view.h"
export module Hash;

export namespace Corvorium {
namespace Hash {
inline constexpr uint64_t fnv1a_64(eastl::string_view Str)
{
    static constexpr uint64_t kOffsetBasis = 0xcbf2'9ce4'8422'2325;
    static constexpr uint64_t kPrime = 0x0000'0100'0000'01b3;
    uint64_t hash = kOffsetBasis;
    for (auto c: Str)
    {
        hash ^= c;
        hash *= kPrime;
    }
    return hash;
}
}

inline constexpr uint64_t operator""_hash(const char* Str, uint64_t Len)
{
    return Corvorium::Hash::fnv1a_64({Str});
}
}
