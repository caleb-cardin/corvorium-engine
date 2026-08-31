// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "slang/slang.h"

module ShaderCompiler:ConversionHelpers;
import :Types;
import RenderingCommon;

namespace Corvorium::ShaderCompiler {
using namespace Corvorium::Rendering;

SlangCompileTarget ToSlangType(OutputIL Il)
{
    switch (Il)
    {
    case OutputIL::SPIRV:
        return SLANG_SPIRV;
    case OutputIL::DXIL:
        return SLANG_DXIL;
    }
    return SLANG_TARGET_UNKNOWN;
}

SlangOptimizationLevel ToSlangType(OptimizationLevel OptLevel)
{
    switch (OptLevel)
    {
    case OptimizationLevel::None:
        return SLANG_OPTIMIZATION_LEVEL_NONE;
    case OptimizationLevel::Default:
        return SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    case OptimizationLevel::High:
        return SLANG_OPTIMIZATION_LEVEL_HIGH;
    case OptimizationLevel::Max:
        return SLANG_OPTIMIZATION_LEVEL_MAXIMAL;
    default:
        return SLANG_OPTIMIZATION_LEVEL_DEFAULT;
    }
}

ShaderResourceType FromSlangType(slang::BindingType Binding)
{
    switch (Binding)
    {
    case slang::BindingType::Sampler:
        return ShaderResourceType::Sampler;
    case slang::BindingType::Texture:
        return ShaderResourceType::SampledTexture;
    case slang::BindingType::ConstantBuffer:
        return ShaderResourceType::UniformBuffer;
    case slang::BindingType::TypedBuffer:
        return ShaderResourceType::UniformTexelBuffer;
    case slang::BindingType::RawBuffer:
        return ShaderResourceType::StorageBuffer;
    case slang::BindingType::CombinedTextureSampler:
        return ShaderResourceType::CombinedTextureSampler;
    case slang::BindingType::InlineUniformData:
        return ShaderResourceType::InlineUniformBlock;
    case slang::BindingType::RayTracingAccelerationStructure:
        return ShaderResourceType::AccelerationStructure;

    case slang::BindingType::MutableTexture:
        return ShaderResourceType::StorageTexture;
    case slang::BindingType::MutableTypedBuffer:
        return ShaderResourceType::StorageTexelBuffer;
    case slang::BindingType::MutableRawBuffer:
        return ShaderResourceType::StorageBuffer;


    case slang::BindingType::PushConstant:
        return ShaderResourceType::PushConstant;

    default:
        CRVM_ASSERT_MSG(false, "Unhandled ShaderResourceType");
        return ShaderResourceType::Sampler;
    }
}

ShaderStageFlagBits FromSlangType(SlangStage InStage)
{
    switch (InStage)
    {
    case SLANG_STAGE_VERTEX:
        return ShaderStageFlagBits::Vertex;
    case SLANG_STAGE_HULL:
        return ShaderStageFlagBits::Hull;
    case SLANG_STAGE_DOMAIN:
        return ShaderStageFlagBits::Domain;
    case SLANG_STAGE_GEOMETRY:
        return ShaderStageFlagBits::Geometry;
    case SLANG_STAGE_FRAGMENT:
        return ShaderStageFlagBits::Fragment;
    case SLANG_STAGE_COMPUTE:
        return ShaderStageFlagBits::Compute;
    case SLANG_STAGE_RAY_GENERATION:
        return ShaderStageFlagBits::RtGen;
    case SLANG_STAGE_INTERSECTION:
        return ShaderStageFlagBits::RtIntersect;
    case SLANG_STAGE_ANY_HIT:
        return ShaderStageFlagBits::RtAnyHit;
    case SLANG_STAGE_CLOSEST_HIT:
        return ShaderStageFlagBits::RtClosestHit;
    case SLANG_STAGE_MISS:
        return ShaderStageFlagBits::RtMiss;
    case SLANG_STAGE_CALLABLE:
        return ShaderStageFlagBits::RtCall;
    case SLANG_STAGE_MESH:
        return ShaderStageFlagBits::Mesh;
    case SLANG_STAGE_AMPLIFICATION:
        return ShaderStageFlagBits::Task;
    case SLANG_STAGE_NONE:
        [[fallthrough]];
    default:
        return ShaderStageFlagBits::None;
    }
}

VertexAttributeEntry::Format::ScalarType FromSlangType(slang::TypeReflection::ScalarType ScalarType)
{
    using OutScalarType = VertexAttributeEntry::Format::ScalarType;
    switch (ScalarType)
    {
    case slang::TypeReflection::Int32:
        return OutScalarType::Int32;
    case slang::TypeReflection::UInt32:
        return OutScalarType::UInt32;
    case slang::TypeReflection::Int64:
        return OutScalarType::Int64;
    case slang::TypeReflection::UInt64:
        return OutScalarType::UInt64;
    case slang::TypeReflection::Float32:
        return OutScalarType::Float32;
    case slang::TypeReflection::Float64:
        return OutScalarType::Float64;
    default:
        return OutScalarType::Unknown;
    }
}
}
