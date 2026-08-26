// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Logging.hpp"

#include "EASTL/vector.h"
#include "EASTL/unique_ptr.h"

#include "vulkan/vulkan.hpp"


module Rendering:VkContext;
import :Context;
import :VkDevice;
import :Types;
import :VkUtils;

import Common;


namespace Corvorium::Rendering::Vulkan {
VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT MessageType,
    const vk::DebugUtilsMessengerCallbackDataEXT* CallbackData,
    [[maybe_unused]] void* UserData)
{
    switch (MessageSeverity)
    {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
        [[fallthrough]];
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
    {
        CRVM_LOG(VulkanLogger, kInfo, "{}", CallbackData->pMessage);
        break;
    }
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
    {
        CRVM_LOG(VulkanLogger, kWarning, "{}", CallbackData->pMessage);
        break;
    }
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
    {
        CRVM_LOG(VulkanLogger, kError, "{}", CallbackData->pMessage);
        break;
    }
    default: { break; }
    }
    return VK_FALSE;
}

class VulkanContext : public RenderContext {
public:
    VulkanContext();
    ~VulkanContext() override;

    [[nodiscard]] RenderError Initialize(OS::Window* MainWindow) override;
    [[nodiscard]] bool IsInitialized() const override;
    [[nodiscard]] RenderDevice* GetDevice() const override;

    void InitializeWindow(OS::Window* Window) override;

private:
    friend class VulkanDevice;
    vk::Instance Instance{nullptr};
    vk::DebugUtilsMessengerEXT DebugMessenger{nullptr};
    eastl::unique_ptr<VulkanDevice> Device{nullptr};

    bool Initialized{false};
};


VulkanContext::VulkanContext()
{}

VulkanContext::~VulkanContext()
{
    if (IsInitialized())
    {
        Device.reset();
        if constexpr (Build::DebugBuild()) { Instance.destroyDebugUtilsMessengerEXT(DebugMessenger); }
        Instance.destroy();
    }
}

RenderError VulkanContext::Initialize(OS::Window* MainWindow)
{
    VulkanDispatcher.init();
    vk::detail::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VulkanDispatcher.init(vkGetInstanceProcAddr);

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
        "VK_EXT_debug_utils",
        "VK_KHR_get_surface_capabilities2"
#endif
    };

#ifndef CORVORIUM_DEBUG
    constexpr u32 instanceExtCount = 3;
#else
    constexpr u32 instanceExtCount = 4;
#endif


#ifdef CORVORIUM_DEBUG
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

    const vk::ResultValue<vk::Instance> result = vk::createInstance(createInfo);
    CRVM_LOG_RETURN_IF(!result.has_value(), RenderError::InitializationError, VulkanLogger, kError, "Failed to create Instance.");
    Instance = result.value;
    VulkanDispatcher.init(Instance);


    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo;
    debugMessengerInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    debugMessengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                     vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debugMessengerInfo.pfnUserCallback = DebugCallback;

    if constexpr (Build::DebugBuild())
    {
        vk::Result res = Instance.createDebugUtilsMessengerEXT(&debugMessengerInfo, nullptr, &DebugMessenger, VulkanDispatcher);
        if (res != vk::Result::eSuccess)
            CRVM_LOG(VulkanLogger, kError, "Failed to create debug messenger.");
    }

    Device = eastl::make_unique<VulkanDevice>(Instance, MainWindow);
    VulkanDispatcher.init(Device->GetDevice());

    Initialized = true;
    return RenderError::Success;
}

bool VulkanContext::IsInitialized() const
{
    return Initialized;
}

RenderDevice* VulkanContext::GetDevice() const
{
    return Device.get();
}

void VulkanContext::InitializeWindow(OS::Window* Window)
{
    Device->CreateSwapchainForWindow(Window);
}
}
