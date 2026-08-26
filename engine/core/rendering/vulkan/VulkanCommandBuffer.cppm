// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Logging.hpp"

#include "vulkan/vulkan.hpp"
module Rendering:VkCommandBuffer;
import :VkDevice;
import :Types;

namespace Corvorium::Rendering::Vulkan {
class VulkanCommandBuffer {
public:
    VulkanCommandBuffer() = delete;

    VulkanCommandBuffer(VulkanDevice* OwningDevice, vk::CommandBuffer Cb)
        : Device{OwningDevice}
        , CmdBuf{Cb}
    {}

    ~VulkanCommandBuffer() = default;

    VulkanCommandBuffer(const VulkanCommandBuffer&) = default;
    VulkanCommandBuffer(VulkanCommandBuffer&&) = default;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = default;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) = default;

    bool Begin()
    {
        vk::CommandBufferBeginInfo info;
        vk::Result res = CmdBuf.begin(info);
        CRVM_LOG_IF(res != vk::Result::eSuccess, VulkanLogger, kError, "Failed to begin command recording");
        return true;
    }

    void BindGraphicsPipeline(PipelineHandle Pipeline)
    {
        vk::Pipeline PipelineToBind = Device->FindPipeline(Pipeline);
        CRVM_LOG_RETURN_IF(PipelineToBind == nullptr, /*void*/;, VulkanLogger, kError, "Failed to bind pipeline: Pipeline not found.");

        CmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, PipelineToBind);
    }

    void DrawMesh(MeshHandle Mesh)
    {}

    bool End()
    {
        vk::Result res = CmdBuf.end();
        CRVM_LOG_IF(res != vk::Result::eSuccess, VulkanLogger, kError, "Failed to begin command recording");
        return true;
    }

private:
    VulkanDevice* Device;
    vk::CommandBuffer CmdBuf;
};
}
