// Copyright © 2026 Caleb Cardin. All rights reserved.
#pragma once

#define DECLARE_FLAG_TYPE_PRIVATE(_flag_name, _enum_type) \
template<> struct FlagTypeTraits<_enum_type> {static constexpr bool IsFlag = true;};\
using _flag_name = FlagType<_enum_type>;\
template _flag_name;

#define DECLARE_FLAG_TYPE(_flag_name, _enum_type) \
template<> struct FlagTypeTraits<_enum_type> {static constexpr bool IsFlag = true;};\
export using _flag_name = FlagType<_enum_type>;\
template _flag_name;
