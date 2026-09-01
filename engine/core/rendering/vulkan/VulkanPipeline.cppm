// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "EASTL/variant.h"

#include "vulkan/vulkan.hpp"
module Rendering:VulkanPipeline;
import :Types;
import :VulkanPipelineUtils;
import :VulkanShaderCache;
import Common;

namespace Corvorium::Rendering::Vulkan {
struct PipelineCreateParamsBase {
    vk::Device InDevice{nullptr};
    vk::PipelineCache PipelineCache{nullptr};
    VulkanShaderCache* ShaderCache{nullptr};
};

struct VulkanGraphicsPipelineCreateParams : public PipelineCreateParamsBase {
    Corvorium::Rendering::GraphicsPipelineCreateParams External{};

};

struct ComputePipelineCreateParams {};

struct RaytracingPipelineCreateParams {};

enum class PipelineType : u8 {
    Graphics,
    Compute,
    Raytracing
};

class VulkanPipeline {
public:
    VulkanPipeline() = default;
    VulkanPipeline(vk::Device InDevice, vk::PipelineCache Cache, const VulkanGraphicsPipelineCreateParams& Params);
    VulkanPipeline(vk::Device InDevice, vk::PipelineCache Cache, const ComputePipelineCreateParams& Params);
    VulkanPipeline(vk::Device InDevice, vk::PipelineCache Cache, const RaytracingPipelineCreateParams& Params);

    VulkanPipeline(const VulkanPipeline& Other) = delete;
    VulkanPipeline(VulkanPipeline&& Other) noexcept = default;
    auto operator=(const VulkanPipeline& Other) -> VulkanPipeline& = delete;
    auto operator=(VulkanPipeline&& Other) noexcept -> VulkanPipeline& = default;

    ~VulkanPipeline();

    [[nodiscard]] vk::Pipeline Get() const { return Pipeline; }
    [[nodiscard]] PipelineType GetType() const { return Type; }

private:
    PipelineType Type;
    vk::Device Device;
    vk::Pipeline Pipeline;
    vk::PipelineLayout Layout;
};


VulkanPipeline::VulkanPipeline(vk::Device InDevice,
                               vk::PipelineCache Cache,
                               const VulkanGraphicsPipelineCreateParams& Params)
    : Type{PipelineType::Graphics}
    , Device{InDevice}
{
    CRVM_ASSERT(Params.External.Module != ShaderModuleHandle::kInvalidHandle);


    const VulkanShader* shader = Params.ShaderCache->GetShader(Params.External.Module);
    CRVM_ASSERT(shader);

    eastl::vector<GraphicsPipelineDescBuilder::ShaderStageInfo> StageInfos;
    StageInfos.reserve(shader->EntryPoints.size());

    for (const auto& entryPoint: shader->EntryPoints)
    {
        StageInfos.emplace_back(GraphicsPipelineDescBuilder::ShaderStageInfo{
            Util::ToVulkanType(entryPoint.Stage), entryPoint.Name.c_str()
        });
    }

    vk::PipelineRenderingCreateInfo renderInfo;
    renderInfo.

    auto builder = GraphicsPipelineDescBuilder()
            .SetRenderingCreateInfo(renderingInfo)
            .AddMultiShader(shader->Module, StageInfos)
            .SetLayout(shader->Layout)
            .SetColorBlends(Params.External.ColorBlend, attachmentCount/*TODO*/)
            .SetPrimitiveType(Params.External.Primitive)
            .SetDepthTestType(Params.External.DepthTest)
            .SetBackfaceCullEnabled(Params.External.BackfaceCullEnabled);

    vk::GraphicsPipelineCreateInfo info = builder.MakeGraphicsPipelineCreateInfo();

    auto result = Device.createGraphicsPipeline(Cache, info);
}


VulkanPipeline::VulkanPipeline(vk::Device InDevice,
                               vk::PipelineCache Cache,
                               const ComputePipelineCreateParams& Params)
    : Type{PipelineType::Compute}
    , Device{InDevice}
{
    vk::GraphicsPipelineCreateInfo info;

    auto result = Device.createGraphicsPipeline(Cache, info);
}


VulkanPipeline::VulkanPipeline(vk::Device InDevice,
                               vk::PipelineCache Cache,
                               const RaytracingPipelineCreateParams& Params)
    : Type{PipelineType::Raytracing}
    , Device{InDevice}
{
    vk::RayTracingPipelineCreateInfoKHR info;

    //auto result = Device.createRayTracingPipelineKHR(Cache, info);
}


VulkanPipeline::~VulkanPipeline()
{
    Device.destroyPipeline(Pipeline);
}
}
