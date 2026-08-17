// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "vulkan/vulkan.hpp"
module Rendering:VkPipeline;

namespace Corvorium::Rendering::Vulkan {

struct GraphicsPipelineCreateParams {

};

class GraphicsPipeline {
public:
    GraphicsPipeline() = delete;
    GraphicsPipeline(vk::Device InDevice, vk::PipelineCache Cache, const GraphicsPipelineCreateParams& Params);

    ~GraphicsPipeline();

private:
    vk::Device Device;
    vk::Pipeline Pipeline;
    vk::PipelineLayout Layout;
};

GraphicsPipeline::GraphicsPipeline(const vk::Device InDevice, const GraphicsPipelineCreateParams& Params)
    : Device{InDevice}
{
    vk::GraphicsPipelineCreateInfo info;

    auto result = Device.createGraphicsPipeline(nullptr, info, );
}

GraphicsPipeline::~GraphicsPipeline()
{
    Device.destroyPipeline(Pipeline);
}
}
