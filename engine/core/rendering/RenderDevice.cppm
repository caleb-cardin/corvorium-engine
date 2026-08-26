// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
export module Rendering:Device;
import OS;
import :CommandBuffer;

export namespace Corvorium::Rendering {
class RenderContext;

class CRVM_API RenderDevice {
public:
    virtual ~RenderDevice() = default;

    [[nodiscard]] virtual bool IsInitialized() const = 0;

    [[nodiscard]] virtual RenderError BeginFrame(OS::WindowHandle Window) = 0;
    [[nodiscard]] virtual ResultValue<CommandBuffer> AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority) = 0;
    [[nodiscard]] virtual RenderError EndFrame(OS::WindowHandle Window) = 0;
};
}
