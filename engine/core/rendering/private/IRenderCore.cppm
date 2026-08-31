// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "EASTL/unique_ptr.h"
module Rendering:RenderCoreImpl;
import :RenderCore;
import :VulkanCore;

namespace Corvorium::Rendering {
eastl::unique_ptr<IRenderCore> IRenderCore::CreateRenderCore(RenderingAPI Api)
{
    switch (Api)
    {
    case RenderingAPI::Auto:
        break;
    case RenderingAPI::Vulkan:
        return eastl::make_unique<Vulkan::VulkanCore>();
    case RenderingAPI::DX12:
        break;
    }
    return nullptr;
}
}
