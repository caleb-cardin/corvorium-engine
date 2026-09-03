// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "vulkan/vulkan.hpp"
module Rendering:VulkanCommandBuffer;
import :VulkanCore;
import :VulkanSwapchain;
import :Types;

namespace Corvorium::Rendering::Vulkan {
class VulkanCore;

class VulkanCommandBuffer {
public:
    VulkanCommandBuffer() = delete;

    VulkanCommandBuffer(VulkanCore* InCore, VulkanSwapchain* InSwapchain, vk::CommandBuffer Cb)
        : Core{InCore}
        , Swapchain{InSwapchain}
        , CmdBuf{Cb}
    {
        CRVM_ASSERT(Core);
        CRVM_ASSERT(Swapchain);
    }

    ~VulkanCommandBuffer() = default;

    VulkanCommandBuffer(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer(VulkanCommandBuffer&&) = default;
    VulkanCommandBuffer& operator=(const VulkanCommandBuffer&) = delete;
    VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) = default;

    [[nodiscard]] bool Begin() const
    {
        vk::CommandBufferBeginInfo info;
        vk::Result res = CmdBuf.begin(info);
        CRVM_LOG_IF(res != vk::Result::eSuccess, VulkanLogger, kError, "Failed to begin command recording");
        return true;
    }

    void BeginRendering() const
    {
        vk::RenderingAttachmentInfo colorAttachment;
        vk::RenderingAttachmentInfo depthAttachment;
        vk::RenderingInfo renderInfo;
        Swapchain->FillRenderingInfo(colorAttachment, depthAttachment, renderInfo);

        CmdBuf.beginRendering(renderInfo);
    }

    void BindGraphicsPipeline(PipelineHandle Pipeline) const
    {
        const VulkanPipeline* PipelineToBind = Core->FindPipeline(Pipeline);
        CRVM_LOG_RETURN_IF(PipelineToBind == nullptr, /*void*/;, VulkanLogger, kError, "Failed to bind pipeline: Pipeline not found.");

        CmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, PipelineToBind->Get());

        const vk::Extent2D scExtent = Swapchain->GetExtent();
        vk::Viewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<f32>(scExtent.width);
        viewport.height = static_cast<f32>(scExtent.height);
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        CmdBuf.setViewport(0, 1, &viewport);

        vk::Rect2D scissor;
        scissor.offset = vk::Offset2D{0, 0};
        scissor.extent = scExtent;

        CmdBuf.setScissor(0, 1, &scissor);
    }

    void TESTING_DrawTriangle()
    {
        CmdBuf.draw(3, 1, 0, 0);
    }

    void DrawMesh(MeshHandle Mesh) const
    {}

    void EndRendering() const
    {
        CmdBuf.endRendering();
    }

    [[nodiscard]] bool End() const
    {
        vk::Result res = CmdBuf.end();
        CRVM_LOG_IF(res != vk::Result::eSuccess, VulkanLogger, kError, "Failed to begin command recording");
        return true;
    }

private:
    VulkanCore* Core{nullptr};
    VulkanSwapchain* Swapchain{nullptr};
    vk::CommandBuffer CmdBuf{nullptr};
};
}
