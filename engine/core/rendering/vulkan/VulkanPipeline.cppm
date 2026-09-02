// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "EASTL/variant.h"

#include "vulkan/vulkan.hpp"
module Rendering:VulkanPipeline;
import :Types;
import :VulkanShaderCache;
import Common;

namespace Corvorium::Rendering::Vulkan {
class VulkanCore;

struct PipelineCreateParamsBase {
    VulkanCore* Core{nullptr};
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
    VulkanPipeline() = delete;
    ResultValue<VulkanPipeline> Create(const VulkanGraphicsPipelineCreateParams& Params);
    ResultValue<VulkanPipeline> Create(const ComputePipelineCreateParams& Params);
    ResultValue<VulkanPipeline> Create(const RaytracingPipelineCreateParams& Params);

    VulkanPipeline(const VulkanPipeline& Other) = delete;
    VulkanPipeline(VulkanPipeline&& Other) noexcept = default;
    auto operator=(const VulkanPipeline& Other) -> VulkanPipeline& = delete;
    auto operator=(VulkanPipeline&& Other) noexcept -> VulkanPipeline& = default;

    ~VulkanPipeline();

    [[nodiscard]] vk::Pipeline Get() const { return Pipeline; }
    [[nodiscard]] PipelineType GetType() const { return Type; }

private:
    PipelineType Type;
    VulkanCore* Core;
    vk::Pipeline Pipeline;
    vk::PipelineLayout Layout;

    VulkanPipeline(PipelineType InType, VulkanCore* InCore, vk::Pipeline InPipeline, vk::PipelineLayout InLayout);
};
}
