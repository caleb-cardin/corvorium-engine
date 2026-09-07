// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "EASTL/variant.h"

module Rendering:VulkanPipeline;
import vulkan;
import :Types;
import :VulkanShaderCache;
import CommonTypes;

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
    static ResultValue<VulkanPipeline> Create(const VulkanGraphicsPipelineCreateParams& Params);
    static ResultValue<VulkanPipeline> Create(const ComputePipelineCreateParams& Params);
    static ResultValue<VulkanPipeline> Create(const RaytracingPipelineCreateParams& Params);

    VulkanPipeline(const VulkanPipeline& Other) = delete;
    VulkanPipeline& operator=(const VulkanPipeline& Other) = delete;

    VulkanPipeline(VulkanPipeline&& Other) noexcept = default;
    VulkanPipeline& operator=(VulkanPipeline&& Other) noexcept = default;

    ~VulkanPipeline();

    [[nodiscard]] PipelineHandle GetHandle() const { return Handle; }
    [[nodiscard]] vk::Pipeline Get() const { return Pipeline; }
    [[nodiscard]] PipelineType GetType() const { return Type; }

private:
    PipelineHandle Handle;
    PipelineType Type;
    Auto<VulkanCore*> Core;
    Auto<vk::Pipeline> Pipeline;
    Auto<vk::PipelineLayout> Layout;

    VulkanPipeline(PipelineType InType, VulkanCore* InCore, vk::Pipeline InPipeline, vk::PipelineLayout InLayout);
};
}
