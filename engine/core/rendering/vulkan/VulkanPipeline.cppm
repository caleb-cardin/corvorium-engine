// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/variant.h"

#include "vulkan/vulkan.hpp"
module Rendering:VulkanPipeline;
import :VulkanPipelineUtils;
import :Types;
import Common;

namespace Corvorium::Rendering::Vulkan {
struct PipelineCreateParamsBase {
    eastl::vector<ShaderHandle> Shaders;
};

struct GraphicsPipelineCreateParams {};

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
    VulkanPipeline(vk::Device InDevice, vk::PipelineCache Cache, const GraphicsPipelineCreateParams& Params);
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
                               const GraphicsPipelineCreateParams& Params)
    : Type{PipelineType::Graphics}
    , Device{InDevice}
{
    vk::GraphicsPipelineCreateInfo info;
    GraphicsPipelineDescBuilder builder;


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
