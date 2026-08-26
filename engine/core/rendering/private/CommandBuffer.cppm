// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/variant.h"
module Rendering:CommandBufferImpl;
import :CommandBuffer;
import :VkCommandBuffer;

#define CB_CALL(_fn, ...) eastl::visit([&](auto& Cb) { Cb._fn(__VA_ARGS__); }, pImpl->CmdBuf)

namespace Corvorium::Rendering {
struct CommandBufferImpl {
    explicit CommandBufferImpl(Vulkan::VulkanCommandBuffer&& VkCb)
        : CmdBuf{VkCb}
    {}

    ~CommandBufferImpl() = default;

    eastl::variant<Vulkan::VulkanCommandBuffer> CmdBuf;
};

CommandBuffer::CommandBuffer(CommandBuffer&&) noexcept = default;
CommandBuffer& CommandBuffer::operator=(CommandBuffer&&) noexcept = default;
CommandBuffer::~CommandBuffer() = default;

CommandBuffer::CommandBuffer(eastl::unique_ptr<CommandBufferImpl>&& Impl)
    : pImpl{eastl::move(Impl)} {}

void CommandBuffer::BindGraphicsPipeline(PipelineHandle Pipeline)
{
    CB_CALL(BindGraphicsPipeline, Pipeline);
}

void CommandBuffer::DrawMesh(MeshHandle Mesh)
{
    CB_CALL(DrawMesh, Mesh);
}
}
