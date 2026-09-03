// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/unique_ptr.h"
export module Rendering:CommandBuffer;
import :Types;


namespace Corvorium::Rendering {
namespace Vulkan {
class VulkanCore;
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

    [[nodiscard]] bool Begin() const;
    void BeginRendering() const;
    void BindGraphicsPipeline(PipelineHandle Pipeline) const;
    void TESTING_DrawTriangle() const;
    void DrawMesh(MeshHandle Mesh) const;
    void EndRendering() const;
    [[nodiscard]] bool End() const;

private:
    friend class Vulkan::VulkanCore;
    eastl::unique_ptr<CommandBufferImpl> pImpl{nullptr};

    explicit CommandBuffer(eastl::unique_ptr<CommandBufferImpl>&& Impl);
};
}
