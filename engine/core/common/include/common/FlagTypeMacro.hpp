// Copyright © 2026 Caleb Cardin. All rights reserved.
#pragma once

#define FLAG_BIT(_bit) = 0x1 << _bit

#define DECLARE_FLAG_TYPE_PRIVATE(_flag_name, _enum_type) \
template<> struct FlagTypeTraits<_enum_type> {static constexpr bool IsFlag = true;};\
using _flag_name = FlagType<_enum_type>;

#define DECLARE_FLAG_TYPE_PRIVATE_NS(_flag_name, _enum_type, _ns) \
template<> struct FlagTypeTraits<_enum_type> {static constexpr bool IsFlag = true;};\
namespace _ns {using _flag_name = FlagType<_enum_type>};

#define DECLARE_FLAG_TYPE(_flag_name, _enum_type) \
template<> struct FlagTypeTraits<_enum_type> {static constexpr bool IsFlag = true;};\
export using _flag_name = FlagType<_enum_type>;

#define DECLARE_FLAG_TYPE_NS(_flag_name, _enum_type, _ns) \
template<> struct FlagTypeTraits<_enum_type> {static constexpr bool IsFlag = true;};\
namespace _ns {export using _flag_name = FlagType<_enum_type>;}
