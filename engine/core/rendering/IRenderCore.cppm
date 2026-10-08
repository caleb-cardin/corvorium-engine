// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "EASTL/unique_ptr.h"
#include "EASTL/string.h"
#include "EASTL/span.h"
#include "EASTL/vector.h"
export module Rendering:RenderCore;
import OS;
import Event;
import RenderingCommon;
import :Types;
import :CommandBuffer;

namespace Corvorium::Rendering {
export enum class RenderingAPI : u8 {
    Auto,
    Vulkan,
    DX12
};

export class IRenderCore {
public:
    virtual ~IRenderCore() = default;

    static eastl::unique_ptr<IRenderCore> CreateRenderCore(RenderingAPI Api);

    [[nodiscard]] virtual RenderError InitializeWindow(OS::Window* Window) = 0;

    [[nodiscard]] virtual RenderError Initialize(Event::GlobalEventBus* EventHandler,
                                                 OS::Window* MainWindow,
                                                 FileSystem::IFileProvider* FileProvider) = 0;
    [[nodiscard]] virtual bool IsInitialized() const = 0;

    // Rendering

    [[nodiscard]] virtual RenderError BeginFrame(OS::WindowHandle Window) = 0;
    [[nodiscard]] virtual ResultValue<CommandBuffer> AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority) = 0;
    [[nodiscard]] virtual RenderError EndFrame(OS::WindowHandle Window) = 0;

    // Shaders

    [[nodiscard]] virtual ResultValue<ShaderHandle> ShaderCreate(const eastl::string& Name) = 0;

    // Pipelines

    [[nodiscard]] virtual ResultValue<PipelineHandle> PipelineCreate(const GraphicsPipelineCreateParams& Params) = 0;

    // Images
    [[nodiscard]] virtual ResultValue<ImageHandle> ImageCreate(Size2D Size, Format Format, ImageUsageFlags Usage) = 0;
    [[nodiscard]] virtual ResultValue<ImageHandle> ImageCreate(Size3D Size, Format Format, ImageUsageFlags Usage) = 0;

    [[nodiscard]] virtual RenderError ImageFillFromMemory(ImageHandle Image, /*element array*/ void* Data, Format DataFormat, u64 Size, u64 Offset = 0) = 0;
    [[nodiscard]] virtual RenderError ImageFillFromMemory(ImageHandle Image, eastl::span<Color> Colors, u64 Offset = 0) = 0;
};
}
