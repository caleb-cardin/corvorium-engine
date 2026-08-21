// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "EASTL/unique_ptr.h"
export module Rendering:Context;
export import Common;
export import :Device;

import OS;
export namespace Corvorium::Rendering {
class RenderDevice;

export enum class RenderingAPI : u8 {
    Auto,
    Vulkan,
    DX12
};

class CRVM_API RenderContext {
public:
    virtual ~RenderContext() = default;

    static eastl::unique_ptr<RenderContext> CreateRenderContext(RenderingAPI Api);

    [[nodiscard]] virtual bool IsInitialized() = 0;
    [[nodiscard]] virtual RenderDevice* GetDevice() const = 0;

    virtual void InitializeWindow(OS::Window* Window) = 0;
};
}
