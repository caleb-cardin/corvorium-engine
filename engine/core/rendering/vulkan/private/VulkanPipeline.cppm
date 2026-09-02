// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "vulkan/vulkan.hpp"
module Rendering:VulkanPipelineImpl;
import :VulkanPipeline;
import :VulkanPipelineUtils;
import :VulkanCore;

namespace Corvorium::Rendering::Vulkan {
ResultValue<VulkanPipeline> VulkanPipeline::Create(const VulkanGraphicsPipelineCreateParams& Params)
{
    CRVM_ASSERT(Params.Core);
    CRVM_ASSERT(Params.ShaderCache);
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

    const Util::SurfaceInfo& surfaceInfo = Core->GetSurfaceInfo();
    vk::PipelineRenderingCreateInfo renderInfo;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &surfaceInfo.ColorFormat.format;
    renderInfo.depthAttachmentFormat = surfaceInfo.DepthStencilFormat;
    renderInfo.stencilAttachmentFormat = surfaceInfo.DepthStencilFormat;

    ResultValue<vk::GraphicsPipelineCreateInfo> result = GraphicsPipelineDescBuilder()
            .SetRenderingCreateInfo(&renderInfo)
            .AddMultiShader(shader->Module, StageInfos)
            .SetLayout(shader->Layout)
            .SetColorBlends(Params.External.ColorBlend, 1)
            .SetPrimitiveType(Params.External.Primitive)
            .SetDepthTestType(Params.External.DepthTest)
            .SetBackfaceCullEnabled(Params.External.BackfaceCullEnabled)
            .Build();

    CRVM_LOG_RETURN_IF(!result.has_value(), ResultError(RenderError::InvalidParameters), VulkanLogger, kError, "Failed to create pipeline");

    auto res = Core->GetDevice().createGraphicsPipeline(Params.PipelineCache, result.value());
    CRVM_LOG_RETURN_IF(!res.has_value(), ResultError(RenderError::InitializationFailed), VulkanLogger, kError, "Failed to create pipeline");

    return VulkanPipeline(PipelineType::Graphics, Params.Core, res.value, shader->Layout);
}


ResultValue<VulkanPipeline> VulkanPipeline::Create(const ComputePipelineCreateParams& Params)
{
    CRVM_ASSERT_MSG(false, "Vulkan compute pipeline creation not implemented");
    return ResultError(RenderError::InternalError);
}


ResultValue<VulkanPipeline> VulkanPipeline::Create(const RaytracingPipelineCreateParams& Params)
{
    CRVM_ASSERT_MSG(false, "Vulkan raytracing pipeline creation not implemented");
    return ResultError(RenderError::InternalError);
}


VulkanPipeline::~VulkanPipeline()
{
    Core->GetDevice().destroyPipeline(Pipeline);
}

VulkanPipeline::VulkanPipeline(PipelineType InType, VulkanCore* InCore, vk::Pipeline InPipeline, vk::PipelineLayout InLayout)
    : Type{InType}
    , Core{InCore}
    , Pipeline{InPipeline}
    , Layout{InLayout}
{}
}
