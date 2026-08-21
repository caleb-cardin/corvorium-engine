// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/FlagTypeMacro.hpp"
export module Rendering:Shader;
import Common;

namespace Corvorium::Rendering {
export enum class ShaderStageFlagBits : u32 {
    None = 0,

    Vertex = 0x1ull << 0,
    TessCtrl = 0x1ull << 1,
    TessEval = 0x1ull << 2,
    Geometry = 0x1ull << 3,
    Fragment = 0x1ull << 4,

    Mesh = 0x1ull << 5,
    Task = 0x1ull << 6,

    RtGen = 0x1ull << 7,
    RtIntersect = 0x1ull << 8,
    RtAnyHit = 0x1ull << 9,
    RtClosestHit = 0x1ull << 10,
    RtMiss = 0x1ull << 11,
    RtCall = 0x1ull << 12,

    Comp = 0x1ull << 13,

    Auto = 0x1ull << 31
};
}

namespace Corvorium {
DECLARE_FLAG_TYPE_NS(ShaderStageFlags, Rendering::ShaderStageFlagBits, Rendering)
}

namespace Corvorium::Rendering {
export class CRVM_API Shader {
public:
    virtual ~Shader() = default;
};
}
