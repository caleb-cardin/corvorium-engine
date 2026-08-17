// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/FlagTypeMacro.hpp"
export module Rendering:Shader;
import Common;
export namespace Corvorium::Rendering {

enum class ShaderStageFlagBits : u32 {
    None = 0,

    Vert = 0x1 << 0,
    TCtrl = 0x1 << 1,
    TEval = 0x1 << 2,
    Geom = 0x1 << 3,
    Frag = 0x1 << 4,

    Mesh = 0x1 << 5,
    Task = 0x1 << 6,

    RGen = 0x1 << 7,
    RInt = 0x1 << 8,
    RAHit = 0x1 << 9,
    RCHit = 0x1 << 10,
    RMiss = 0x1 << 11,
    RCall = 0x1 << 12,

    Comp = 0x1 << 13,

    Auto = 0x1 << 31
};
DECLARE_FLAG_TYPE(ShaderStageFlags, ShaderStageFlagBits)

class Shader {
public:
    virtual ~Shader() = default;


};

}
