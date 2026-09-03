// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "EASTL/unique_ptr.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"
export module Rendering:RenderCore;
import Common;
import OS;
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

    [[nodiscard]] virtual RenderError Initialize(OS::Window* MainWindow, FileSystem::IFileProvider* FileProvider) = 0;
    [[nodiscard]] virtual bool IsInitialized() const = 0;
    [[nodiscard]] virtual RenderError BeginFrame(OS::WindowHandle Window) = 0;
    [[nodiscard]] virtual ResultValue<CommandBuffer> AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority) = 0;
    [[nodiscard]] virtual RenderError EndFrame(OS::WindowHandle Window) = 0;

    [[nodiscard]] virtual ResultValue<ShaderModuleHandle> CreateShaderModule(const eastl::string& Name) = 0;
    [[nodiscard]] virtual ResultValue<PipelineHandle> CreatePipeline(const GraphicsPipelineCreateParams& Params) = 0;
};
}
