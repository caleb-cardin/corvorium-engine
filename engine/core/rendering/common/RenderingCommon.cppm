// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/FlagTypeMacro.hpp"

#include "EASTL/optional.h"
#include "EASTL/string.h"
export module RenderingCommon;
import Common;

namespace Corvorium::Rendering {
export enum class ShaderStageFlagBits : u32 {
    None = 0,

    Vertex FLAG_BIT(0),
    Hull FLAG_BIT(1),
    Domain FLAG_BIT(2),
    Geometry FLAG_BIT(3),
    Fragment FLAG_BIT(4),

    Mesh FLAG_BIT(5),
    Task FLAG_BIT(6),

    RtGen FLAG_BIT(7),
    RtIntersect FLAG_BIT(8),
    RtAnyHit FLAG_BIT(9),
    RtClosestHit FLAG_BIT(10),
    RtMiss FLAG_BIT(11),
    RtCall FLAG_BIT(12),

    Compute FLAG_BIT(13),
};
}

namespace Corvorium {
DECLARE_FLAG_TYPE_NS(ShaderStageFlags, Rendering::ShaderStageFlagBits, Rendering)
}

namespace Corvorium::Rendering {
export enum class ShaderResourceType {
    Unknown,
    // read only constant buffer
    UniformBuffer,
    // read-write mutable buffer
    StorageBuffer,
    // read only typed constant buffer
    UniformTexelBuffer,
    // read-write typed mutable buffer
    StorageTexelBuffer,
    // read only, sampled image
    SampledTexture,
    // read-write image
    StorageTexture,
    // inline uniform data
    InlineUniformBlock,
    // standalone sampler
    Sampler,
    // texture & sampler
    CombinedTextureSampler,
    // push-constant data
    PushConstant,
    // raytracing acceleration structure
    AccelerationStructure
};


export struct CRVM_API ShaderBindingEntry {
    eastl::string Name{};
    ShaderResourceType Type{};
    u32 Set{};
    u32 Binding{};
    eastl::optional<u32> Count{};
    eastl::optional<u32> ElementSize{};
    ShaderStageFlags Stage{ShaderStageFlagBits::None};
};

export struct CRVM_API ShaderPushConstantEntry {
    eastl::string Name{};
    u32 Offset{};
    u32 Size{};
    ShaderStageFlags Stage{ShaderStageFlagBits::None};
};

export struct CRVM_API VertexAttributeEntry {
    struct Format {
        u8 ComponentCount;
        u8 ComponentSize;

        enum class ScalarType : u8 {
            Unknown,
            Int32,
            UInt32,
            Int64,
            UInt64,
            Float32,
            Float64
        } Type;
    };

    enum class SemanticType : u8 {
        Position,
        Normal,
        TextureCoord,
        Color,
    };

    u32 Location{};
    Format Format{};
    SemanticType Semantic{};
    u32 SemanticIndex{};
};
}
