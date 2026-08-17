
module;

#include "EASTL/vector.h"
#include "vulkan/vulkan.hpp"
module Rendering:VkContext;
import :Context;

import Common;


namespace Corvorium::Rendering::Vulkan {

class VulkanContext : public RenderContext {
public:
    VulkanContext() = delete;
    ~VulkanContext() override = default;

    bool Initialize() override;
    void Shutdown() override;

private:
    friend class VulkanDevice;
    vk::Instance Instance;

    bool IsInitialized = false;
};


bool VulkanContext::Initialize()
{

    vk::ApplicationInfo appInfo;
    appInfo.apiVersion = vk::makeApiVersion(0, 1, 4, 0);
    appInfo.pApplicationName = ""; // TODO
    appInfo.applicationVersion = vk::makeApiVersion(0, CORVORIUM_VERSION_MAJOR, CORVORIUM_VERSION_MINOR, CORVORIUM_VERSION_PATCH);
    appInfo.pEngineName = "Corvorium Engine";
    appInfo.engineVersion = vk::makeApiVersion(0, CORVORIUM_VERSION_MAJOR, CORVORIUM_VERSION_MINOR, CORVORIUM_VERSION_PATCH);

    constexpr const char* instanceExtensions[] = {
        "VK_KHR_surface",
#ifdef CORVORIUM_WINDOWS
        "VK_KHR_win32_surface",
#else
        "VK_KHR_wayland_surface",
#endif
#ifdef CORVORIUM_DEBUG
        "VK_EXT_debug_utils"
#endif
    };

#ifndef CORVORIUM_DEBUG
    constexpr u32 instanceExtCount = 2;
#else
    constexpr u32 instanceExtCount = 3;
#endif


#ifndef CORVORIUM_DEBUG
    constexpr u32 instanceLayerCount = 1;
    constexpr const char* instanceLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
#else
    constexpr u32 instanceLayerCount = 0;
    constexpr const char** instanceLayers = nullptr;
#endif

    vk::InstanceCreateInfo createInfo;
    createInfo.enabledExtensionCount = instanceExtCount;
    createInfo.ppEnabledExtensionNames = instanceExtensions;
    createInfo.enabledLayerCount = instanceLayerCount;
    createInfo.ppEnabledLayerNames = instanceLayers;
    createInfo.pApplicationInfo = &appInfo;


    if (const vk::ResultValue<vk::Instance> result = vk::createInstance(createInfo); result.has_value())
    {
        Instance = result.value;
        IsInitialized = true;
    }
    else
    {
        return false;
    }



    return true;
}

void VulkanContext::Shutdown()
{}

}
