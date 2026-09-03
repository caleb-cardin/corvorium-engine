// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/variant.h"
module Rendering:CommandBufferImpl;
import :CommandBuffer;
import :VulkanCommandBuffer;

#define CB_CALL(_fn, ...) eastl::visit([&](auto& Cb) { Cb._fn(__VA_ARGS__); }, pImpl->CmdBuf)
#define CB_CALL_RET(_fn, ...) eastl::visit([&](auto& Cb) { return Cb._fn(__VA_ARGS__); }, pImpl->CmdBuf)

namespace Corvorium::Rendering {
struct CommandBufferImpl {
public:
    explicit CommandBufferImpl(Vulkan::VulkanCommandBuffer&& VkCb)
        : CmdBuf{eastl::move(VkCb)}
    {}

    ~CommandBufferImpl() = default;

    eastl::variant<Vulkan::VulkanCommandBuffer> CmdBuf;
};


CommandBuffer::CommandBuffer(CommandBuffer&&) noexcept = default;
CommandBuffer& CommandBuffer::operator=(CommandBuffer&&) noexcept = default;

CommandBuffer::~CommandBuffer() = default;

CommandBuffer::CommandBuffer(eastl::unique_ptr<CommandBufferImpl>&& Impl)
    : pImpl{eastl::move(Impl)} {}

bool CommandBuffer::Begin() const
{
    return CB_CALL_RET(Begin);
}

void CommandBuffer::BeginRendering() const
{
    CB_CALL(BeginRendering);
}

void CommandBuffer::EndRendering() const
{
    CB_CALL(EndRendering);
}

void CommandBuffer::BindGraphicsPipeline(PipelineHandle Pipeline) const
{
    CB_CALL(BindGraphicsPipeline, Pipeline);
}

void CommandBuffer::TESTING_DrawTriangle() const
{
    CB_CALL(TESTING_DrawTriangle);
}

void CommandBuffer::DrawMesh(MeshHandle Mesh) const
{
    CB_CALL(DrawMesh, Mesh);
}

bool CommandBuffer::End() const
{
    return CB_CALL_RET(End);
}
}
