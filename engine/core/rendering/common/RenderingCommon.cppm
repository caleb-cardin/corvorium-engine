// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/FlagTypeMacro.hpp"

#include "EASTL/optional.h"
#include "EASTL/string.h"
export module RenderingCommon;
import CommonTypes;
import Flags;

export import :Primitives;

namespace Corvorium {
namespace Rendering {
export struct CRVM_API Size2D {
    u32 X;
    u32 Y;
};

export struct CRVM_API Size3D {
    u32 X;
    u32 Y;
    u32 Z;
};

export enum class ImageUsageFlagBits : u8 {
    None = 0,
    ColorAttachment FLAG_BIT(0),
    DepthStencilAttachment FLAG_BIT(1),
    ShaderResource FLAG_BIT(2),
    Storage FLAG_BIT(3),
    TransferDst FLAG_BIT(4),
    TransferSrc FLAG_BIT(5),

    TransferMask = TransferDst | TransferSrc
};
}

DECLARE_FLAG_TYPE_NS(ImageUsageFlags, Rendering::ImageUsageFlagBits, Rendering);

namespace Rendering {
export enum class ShaderStageFlagBits : u16 {
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

DECLARE_FLAG_TYPE_NS(ShaderStageFlags, Rendering::ShaderStageFlagBits, Rendering)

namespace Rendering {
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


export struct alignas(4) CRVM_API Format {
    // The following enums use underlying values to decode into an indexed array.
    // If any of the following values, the conversion function(s) must also be changed.

    enum class ComponentCount : u8 {
        e1 = 1,
        e2 = 2,
        e3 = 3,
        e4 = 4,
        Unknown = static_cast<u8>(~0x0),
    } Count{ComponentCount::Unknown};
    enum class ComponentSize : u8 {
        e8 = 8,
        e16 = 16,
        e32 = 32,
        e64 = 64,
        Unknown = static_cast<u8>(~0x0),
    } Size{ComponentSize::Unknown};
    enum class ScalarType : u8 {
        U = 0, // unsigned int
        I = 1, // signed int
        Un = 2, // unsigned norm
        In = 3, // signed norm
        Us = 4, // unsigned scaled
        Is = 5, // signed scaled
        F = 6, // float
        Unknown = static_cast<u8>(~0x0),
    } Type{ScalarType::Unknown};
};

export enum class SemanticType : u8 {
    Position,
    Normal,
    TextureCoord,
    Color,
};

export struct CRVM_API VertexAttributeEntry {
    u32 Location{};
    Format Format{};
    SemanticType Semantic{};
    u32 SemanticIndex{};
};

}
}
