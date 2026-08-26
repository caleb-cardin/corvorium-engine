// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/unique_ptr.h"
export module Rendering:CommandBuffer;
import :Types;


namespace Corvorium::Rendering {
namespace Vulkan {
class VulkanDevice;
}

struct CommandBufferImpl;

export class CommandBuffer {
public:
    CommandBuffer() = delete;
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer(CommandBuffer&&) noexcept;
    CommandBuffer& operator=(const CommandBuffer&) = delete;
    CommandBuffer& operator=(CommandBuffer&&) noexcept;
    ~CommandBuffer();

    void Begin();
    void BindGraphicsPipeline(PipelineHandle Pipeline);
    void DrawMesh(MeshHandle Mesh);
    void End();

private:
    friend class Vulkan::VulkanDevice;
    eastl::unique_ptr<CommandBufferImpl> pImpl{nullptr};

    explicit CommandBuffer(eastl::unique_ptr<CommandBufferImpl>&& Impl);
};
}
