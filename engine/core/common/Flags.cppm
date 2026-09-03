// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include <type_traits>
export module Flags;
export import TypeDefs;
export namespace Corvorium {
template<typename E>
struct FlagTypeTraits {
    static constexpr bool IsFlag = false;
};

template<typename E>
concept FlagBitEnum = std::is_scoped_enum_v<E> && FlagTypeTraits<E>::IsFlag;

template<FlagBitEnum E>
class FlagType {
public:
    using Eut = std::underlying_type_t<E>;

    constexpr FlagType() = default;

    constexpr FlagType(E Flagbit)
        : FlagBits{static_cast<Eut>(Flagbit)}
    {}

    constexpr explicit FlagType(Eut Flagbit)
        : FlagBits{static_cast<Eut>(Flagbit)}
    {}

    ~FlagType() = default;

    constexpr operator bool() const
    {
        return FlagBits;
    }

    constexpr FlagType(const FlagType& Other) = default;
    constexpr FlagType(FlagType&& Other) = default;

    constexpr FlagType& operator=(const FlagType& Other) = default;
    constexpr FlagType& operator=(FlagType&& Other) = default;

    constexpr bool operator==(const FlagType& Other) const
    {
        return FlagBits == Other.FlagBits;
    }

    constexpr bool operator==(E Other) const
    {
        return FlagBits == static_cast<Eut>(Other);
    }

    constexpr FlagType operator~() const
    {
        return FlagType{static_cast<Eut>(~FlagBits)};
    }

    constexpr FlagType operator&(const E Flag) const
    {
        return FlagType{static_cast<Eut>(FlagBits & static_cast<Eut>(Flag))};
    }

    constexpr FlagType operator&(const FlagType Other) const
    {
        return FlagType{static_cast<Eut>(FlagBits & Other.FlagBits)};
    }

    constexpr FlagType operator|(const E Flag) const
    {
        return FlagType{static_cast<Eut>(FlagBits | static_cast<Eut>(Flag))};
    }

    constexpr FlagType operator|(const FlagType Other) const
    {
        return FlagType{static_cast<Eut>(FlagBits | Other.FlagBits)};
    }

    constexpr FlagType operator^(const E Flag) const
    {
        return FlagType{static_cast<Eut>(FlagBits ^ static_cast<Eut>(Flag))};
    }

    constexpr FlagType operator^(const FlagType Other) const
    {
        return FlagType{static_cast<Eut>(FlagBits ^ Other.FlagBits)};
    }

    constexpr FlagType& operator&=(const E Flag)
    {
        FlagBits &= static_cast<Eut>(Flag);
        return *this;
    }

    constexpr FlagType& operator&=(const FlagType Other)
    {
        FlagBits &= Other.FlagBits;
        return *this;
    }

    constexpr FlagType& operator|=(const E Flag)
    {
        FlagBits |= static_cast<Eut>(Flag);
        return *this;
    }

    constexpr FlagType& operator|=(const FlagType Other)
    {
        FlagBits |= Other.FlagBits;
        return *this;
    }

    constexpr FlagType& operator^=(const E Flag)
    {
        FlagBits ^= static_cast<Eut>(Flag);
        return *this;
    }

    constexpr FlagType& operator^=(const FlagType Other)
    {
        FlagBits ^= Other.FlagBits;
        return *this;
    }

private:
    Eut FlagBits{0};
};

template<FlagBitEnum E>
constexpr FlagType<E> operator==(const E Lhs, const E Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) == static_cast<FlagType<E>::Eut>(Rhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator==(const E Lhs, const FlagType<E> Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) == static_cast<FlagType<E>::Eut>(Rhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator~(const E Lhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(~static_cast<FlagType<E>::Eut>(Lhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator&(const E Lhs, const E Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) & static_cast<FlagType<E>::Eut>(Rhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator&(const E Lhs, const FlagType<E> Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) & static_cast<FlagType<E>::Eut>(Rhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator|(const E Lhs, const E Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) | static_cast<FlagType<E>::Eut>(Rhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator|(const E Lhs, const FlagType<E> Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) | static_cast<FlagType<E>::Eut>(Rhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator^(const E Lhs, const E Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) ^ static_cast<FlagType<E>::Eut>(Rhs))};
}

template<FlagBitEnum E>
constexpr FlagType<E> operator^(const E Lhs, const FlagType<E> Rhs)
{
    return FlagType<E>{static_cast<FlagType<E>::Eut>(static_cast<FlagType<E>::Eut>(Lhs) ^ static_cast<FlagType<E>::Eut>(Rhs))};
}
}
