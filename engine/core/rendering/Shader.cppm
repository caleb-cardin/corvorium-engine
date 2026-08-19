// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/FlagTypeMacro.hpp"
export module Rendering:Shader;
import Common;

namespace Corvorium::Rendering {
export enum class ShaderStageFlagBits : u32 {
    None = 0,

    Vertex = 0x1 << 0,
    TessCtrl = 0x1 << 1,
    TessEval = 0x1 << 2,
    Geometry = 0x1 << 3,
    Fragment = 0x1 << 4,

    Mesh = 0x1 << 5,
    Task = 0x1 << 6,

    RtGen = 0x1 << 7,
    RtIntersect = 0x1 << 8,
    RtAnyHit = 0x1 << 9,
    RtClosestHit = 0x1 << 10,
    RtMiss = 0x1 << 11,
    RtCall = 0x1 << 12,

    Comp = 0x1 << 13,

    Auto = 0x1 << 31
};
}

namespace Corvorium {
DECLARE_FLAG_TYPE_NS(ShaderStageFlags, Rendering::ShaderStageFlagBits, Rendering)
}

namespace Corvorium::Rendering {
export class Shader {
public:
    virtual ~Shader() = default;
};
}
