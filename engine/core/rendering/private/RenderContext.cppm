// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "EASTL/unique_ptr.h"
module Rendering:ContextImpl;
import :Context;
import :VkContext;

namespace Corvorium::Rendering {
eastl::unique_ptr<RenderContext> RenderContext::CreateRenderContext(RenderingAPI Api)
{
    switch (Api)
    {
    case RenderingAPI::Auto:
        break;
    case RenderingAPI::Vulkan:
        return eastl::make_unique<Vulkan::VulkanContext>();
    case RenderingAPI::DX12:
        break;
    }
    return nullptr;
}
}
