// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "EASTL/hash_map.h"
#include "EASTL/string.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/variant.h"

#include "vulkan/vulkan.hpp"
#include "vma/vk_mem_alloc.h"
module Rendering:VulkanCoreImpl;
import :VulkanCore;
import :CommandBufferImpl;
import :VulkanCommandBuffer;
import :VulkanSwapchain;
import :VulkanUtils;
import Build;

namespace Corvorium::Rendering::Vulkan {
VulkanCore::~VulkanCore()
{
    Teardown();
}

RenderError VulkanCore::Initialize(OS::Window* MainWindow, FileSystem::IFileProvider* InFileProvider)
{
    CRVM_ASSERT(MainWindow);
    CRVM_ASSERT(InFileProvider);

    FileProvider = InFileProvider;

    if (!InitInstance())
    {
        Teardown();
        return RenderError::InitializationFailed;
    }

    if (!SelectPhysicalDevice())
    {
        Teardown();
        return RenderError::InitializationFailed;
    }

    vk::SurfaceKHR MainSurface = Util::CreateWindowSurface(Instance, MainWindow);
    if (!SelectSurfaceFormats(MainSurface))
    {
        Instance.destroySurfaceKHR(MainSurface);
        Teardown();
        return RenderError::InitializationFailed;
    }
    bool PresentFound = FindPresentQueueFamily(MainSurface, PresentQueueFamily);
    if (!PresentFound)
    {
        Instance.destroySurfaceKHR(MainSurface);
        Teardown();
        return RenderError::InitializationFailed;
    }

    auto& scElement = Swapchains.emplace_back_unsorted(MainWindow->GetHandle(), VulkanSwapchain::CreateUninitializedSwapchain(MainSurface));

    if (!InitDevice())
    {
        Teardown();
        return RenderError::InitializationFailed;
    }

    ShaderCache = eastl::make_unique<VulkanShaderCache>(Device, FileProvider);

    bool swapchainInitialized = scElement.second.Initialize(VulkanSwapchainCreateParams{
        .Instance = Instance,
        .Core = this,
        .Window = MainWindow,
        .GraphicsQueue = GraphicsQueue.FamilyIndex,
        .PresentQueue = PresentQueueFamily
    });

    if (!swapchainInitialized)
    {
        Teardown();
        return RenderError::InitializationFailed;
    }

    Initialized = true;
    return RenderError::Success;
}

RenderError VulkanCore::InitializeWindow(OS::Window* Window)
{
    if (Swapchains.contains(Window->GetHandle()))
    {
        return RenderError::AlreadyInitialized;
    }

    VulkanSwapchainCreateParams params{
        .Instance = Instance,
        .Core = this,
        .Window = Window,
        .GraphicsQueue = GraphicsQueue.FamilyIndex,
        .PresentQueue = PresentQueueFamily
    };

    Swapchains.emplace_back_unsorted(Window->GetHandle(), VulkanSwapchain::CreateInitializedSwapchain(params));

    return RenderError::Success;
}

RenderError VulkanCore::BeginFrame(OS::WindowHandle Window)
{
    auto it = Swapchains.find(Window);
    CRVM_LOG_RETURN_IF(it == Swapchains.end(),
                       RenderError::InvalidHandle,
                       VulkanLogger,
                       kError,
                       "Invalid window handle passed to RenderDevice::BeginFrame()");

    VulkanSwapchain& swapchain = it->second;

    if (swapchain.IsMinimized()) return RenderError::WindowMinimized;

    vk::Result res = swapchain.WaitForFence();
    CRVM_ASSERT(res == vk::Result::eSuccess);

    if (swapchain.IsOutOfDate())
    {
        if (swapchain.CreateOrRecreateSwapchain() == RenderError::WindowMinimized)
        {
            return RenderError::WindowMinimized;
        }
    }

    res = swapchain.AcquireImage();
    if (res == vk::Result::eErrorOutOfDateKHR)
    {
        return RenderError::SwapchainOOD;
    }
    else if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR)
    {
        CRVM_ASSERT_MSG(false, "BeginFrame() failed.");
        return RenderError::InternalError;
    }

    res = swapchain.Reset();
    CRVM_ASSERT(res == vk::Result::eSuccess);

    res = swapchain.InitializeCommandList();
    CRVM_ASSERT(res == vk::Result::eSuccess);
    return RenderError::Success;
}

ResultValue<CommandBuffer> VulkanCore::AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority)
{
    auto it = Swapchains.find(Window);
    CRVM_LOG_RETURN_IF(it == Swapchains.end(),
                       ResultError(RenderError::InvalidHandle),
                       VulkanLogger,
                       kError,
                       "Invalid window handle passed to RenderDevice::EndFrame()");

    VulkanSwapchain& swapchain = it->second;

    if (swapchain.IsMinimized()) return ResultError(RenderError::WindowMinimized);
    if (swapchain.IsOutOfDate()) return ResultError(RenderError::SwapchainOOD);

    vk::CommandBuffer cb;
    vk::Result res = swapchain.AllocateCommandBuffer(Priority, cb);
    CRVM_ASSERT(res == vk::Result::eSuccess);

    return CommandBuffer{eastl::make_unique<CommandBufferImpl>(VulkanCommandBuffer{this, &swapchain, cb})};
}

RenderError VulkanCore::EndFrame(OS::WindowHandle Window)
{
    auto it = Swapchains.find(Window);
    CRVM_LOG_RETURN_IF(it == Swapchains.end(),
                       RenderError::InvalidHandle,
                       VulkanLogger,
                       kError,
                       "Invalid window handle passed to RenderDevice::EndFrame()");

    VulkanSwapchain& swapchain = it->second;

    vk::Result res = swapchain.CompleteFrame(GraphicsQueue.Queue);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, RenderError::InternalError, VulkanLogger, kError, "EndFrame failed.");
    return RenderError::Success;
}

ResultValue<ShaderModuleHandle> VulkanCore::CreateShaderModule(const eastl::string& Name)
{
    auto result = ShaderCache->GetOrCreateShader(Name);
    if (result.has_value())
    {
        return result.value()->Handle;
    }
    return ResultError(RenderError::InternalError);
}

ResultValue<PipelineHandle> VulkanCore::CreatePipeline(const GraphicsPipelineCreateParams& Params)
{
    VulkanGraphicsPipelineCreateParams internalParams{};
    internalParams.Core = this;
    internalParams.ShaderCache = ShaderCache.get();
    internalParams.PipelineCache = {nullptr};
    internalParams.External = Params;

    ResultValue<VulkanPipeline> pipelineRes = VulkanPipeline::Create(internalParams);
    CRVM_LOG_RETURN_IF(!pipelineRes.has_value(), ResultError(RenderError::InternalError), VulkanLogger, kError, "Pipeline creation failed.");

    auto [it, success] = Pipelines.try_emplace(pipelineRes.value().GetHandle(), eastl::move(pipelineRes.value()));
    CRVM_ASSERT(success);

    return it->second.GetHandle();
}

void VulkanCore::OnWindowClosed(OS::Window* Window, void* OwningDevice)
{
    CRVM_ASSERT(Window);
    CRVM_ASSERT(OwningDevice);
    auto* Device = static_cast<VulkanCore*>(OwningDevice);
    const OS::WindowHandle handle = Window->GetHandle();
    if (Device->Swapchains.contains(handle))
    {
        Device->Swapchains.erase(handle);
    }
}

const VulkanPipeline* VulkanCore::FindPipeline(PipelineHandle Pipeline)
{
    auto it = Pipelines.find(Pipeline);
    if (it != Pipelines.end())
    {
        return &it->second;
    }
    return nullptr;
}

bool VulkanCore::InitInstance()
{
    VulkanDispatcher.init();
    vk::detail::DynamicLoader dl;
    auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VulkanDispatcher.init(vkGetInstanceProcAddr);

    vk::ApplicationInfo appInfo;
    appInfo.apiVersion = vk::ApiVersion14;
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
    CRVM_LOG_RETURN_IF(!result.has_value(), false, VulkanLogger, kError, "Failed to create Instance.");
    Instance = result.value;
    VulkanDispatcher.init(Instance);

    vk::DebugUtilsMessengerCreateInfoEXT debugMessengerInfo;
    debugMessengerInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;
    debugMessengerInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
                                     vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    debugMessengerInfo.pfnUserCallback = Util::DebugCallback;

    if constexpr (Build::DebugBuild())
    {
        vk::Result res = Instance.createDebugUtilsMessengerEXT(&debugMessengerInfo, nullptr, &DebugMessenger, VulkanDispatcher);
        if (res != vk::Result::eSuccess)
            CRVM_LOG(VulkanLogger, kError, "Failed to create debug messenger.");
    }

    return true;
}

bool VulkanCore::SelectPhysicalDevice()
{
    u32 deviceCount = 0;
    vk::Result res = Instance.enumeratePhysicalDevices(&deviceCount, nullptr);
    CRVM_LOG_RETURN_IF(deviceCount == 0 || res != vk::Result::eSuccess,
                       false,
                       VulkanLogger,
                       kError,
                       "Failed to initialize device: No GPU found.");

    eastl::vector<vk::PhysicalDevice> physicalDevices;
    physicalDevices.resize(deviceCount);
    res = Instance.enumeratePhysicalDevices(&deviceCount, physicalDevices.data());

    eastl::vector<u32> deviceScores(physicalDevices.size(), 0);
    for (u32 i = 0; const auto& device: physicalDevices)
    {
        vk::PhysicalDeviceProperties2 props;
        device.getProperties2(&props);
        if (props.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
        {
            deviceScores[i] += 1000;
        }
        else if (props.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
        {
            deviceScores[i] += 100;
        }
        else if (props.properties.deviceType == vk::PhysicalDeviceType::eCpu)
        {
            deviceScores[i] += 50;
        }

        i++;
    }

    vk::PhysicalDevice chosenDevice = nullptr;
    u32 chosenScore = 0;
    for (u32 i = 0; auto score: deviceScores)
    {
        if (score > chosenScore)
        {
            chosenDevice = physicalDevices[i];
            chosenScore = score;
        }
        i++;
    }

    PhysicalDevice = chosenDevice;
    return true;
}

static vk::SurfaceFormat2KHR SelectOptimalFormat(const eastl::vector<vk::SurfaceFormat2KHR>& Formats)
{
    for (const auto& format: Formats)
    {
        if (format.surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && format.surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return format;
        }
    }
    return Formats[0];
}

static vk::PresentModeKHR SelectOptimalPresentMode(const eastl::vector<vk::PresentModeKHR>& Modes)
{
    for (const auto& mode: Modes) { if (mode == vk::PresentModeKHR::eMailbox) return mode; }
    return vk::PresentModeKHR::eFifo;
}

bool VulkanCore::SelectSurfaceFormats(vk::SurfaceKHR Surface)
{
    CRVM_ASSERT(Surface);
    CRVM_ASSERT(PhysicalDevice);

    vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo;
    surfaceInfo.surface = Surface;
    vk::SurfaceCapabilities2KHR surfaceCaps;
    vk::Result res = PhysicalDevice.getSurfaceCapabilities2KHR(&surfaceInfo, &surfaceCaps, VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface capabilities.");

    u32 formatCount = 0;
    res = PhysicalDevice.getSurfaceFormats2KHR(&surfaceInfo, &formatCount, nullptr, VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface formats.");
    eastl::vector<vk::SurfaceFormat2KHR> surfaceFormats{formatCount, vk::SurfaceFormat2KHR{}};
    res = PhysicalDevice.getSurfaceFormats2KHR(&surfaceInfo, &formatCount, surfaceFormats.data(), VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface formats.");

    const vk::SurfaceFormat2KHR selectedFormat = SelectOptimalFormat(surfaceFormats);
    SurfaceInfo.ColorFormat = selectedFormat.surfaceFormat;

    u32 presentModeCount = 0;
    res = PhysicalDevice.getSurfacePresentModesKHR(Surface, &presentModeCount, nullptr, VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface present modes.");
    eastl::vector<vk::PresentModeKHR> surfacePresentModes{presentModeCount, vk::PresentModeKHR{}};
    res = PhysicalDevice.getSurfacePresentModesKHR(Surface, &presentModeCount, surfacePresentModes.data(), VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface present modes.");

    SurfaceInfo.PresentMode = SelectOptimalPresentMode(surfacePresentModes);

    static constexpr eastl::array<vk::Format, 3> kDepthFormats{
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint
    };
    bool depthStencilFound = false;
    for (const auto depthFormat: kDepthFormats)
    {
        vk::FormatProperties props = PhysicalDevice.getFormatProperties(depthFormat);
        if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            SurfaceInfo.DepthStencilFormat = depthFormat;
            depthStencilFound = true;
            break;
        }
    }
    CRVM_ASSERT_MSG(depthStencilFound, "Depth/Stencil formats unsupported!");
    return true;
}

bool VulkanCore::InitDevice()
{
    eastl::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    auto queueInfos = CreateQueueInfos(queueCreateInfos);
    CRVM_LOG_RETURN_IF(queueCreateInfos.size() != 2, false, VulkanLogger, kError, "Failed to query for queue data.");
    GraphicsQueue = queueInfos[0];
    TransferQueue = queueInfos[1];

    vk::DeviceCreateInfo info;
    info.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
    info.pQueueCreateInfos = queueCreateInfos.data();

    // Extensions

    constexpr eastl::array<const char*, 1> reqDeviceExtNames{
        "VK_KHR_swapchain"
    };
    constexpr const eastl::array<const char*, 9> optDeviceExtNames{
        "VK_EXT_memory_budget",
        "VK_EXT_memory_priority",
        "VK_AMD_device_coherent_memory",
        "VK_EXT_descriptor_heap",
        "VK_KHR_maintenance7",
        "VK_KHR_deferred_host_operations",
        "VK_KHR_acceleration_structure",
        "VK_KHR_ray_tracing_pipeline",
        "VK_EXT_mesh_shader"
    };

    u32 propertyCount = 0;
    vk::Result res = PhysicalDevice.enumerateDeviceExtensionProperties(nullptr, &propertyCount, nullptr);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to enumerate physical devices.");
    eastl::vector<vk::ExtensionProperties> properties(propertyCount, vk::ExtensionProperties{});
    res = PhysicalDevice.enumerateDeviceExtensionProperties(nullptr, &propertyCount, properties.data());
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to enumerate physical devices.");
    // TODO check results

    auto checkOptExtName = [optDeviceExtNames](const eastl::vector<vk::ExtensionProperties>& available) -> eastl::vector<const char*> {
        eastl::vector<const char*> extensions;
        for (const auto& prop: available)
        {
            for (const char* checkName: optDeviceExtNames)
            {
                if (std::strcmp(prop.extensionName, checkName) == 0)
                {
                    extensions.emplace_back(checkName);
                }
            }
        }
        return extensions;
    };

    eastl::vector<const char*> extensions = checkOptExtName(properties);
    extensions.insert(extensions.end(), reqDeviceExtNames.begin(), reqDeviceExtNames.end());

    info.enabledExtensionCount = static_cast<u32>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();

    // Features

    vk::PhysicalDeviceFeatures2 coreFeatures;
    vk::PhysicalDeviceVulkan11Features features11;
    vk::PhysicalDeviceVulkan12Features features12;
    vk::PhysicalDeviceVulkan13Features features13;
    vk::PhysicalDeviceVulkan14Features features14;
    vk::PhysicalDeviceDynamicRenderingFeatures drFeatures;
    coreFeatures.pNext = &features11;
    features11.pNext = &features12;
    features12.pNext = &features13;
    features13.pNext = &features14;
    features14.pNext = &drFeatures;

    PhysicalDevice.getFeatures2(&coreFeatures);
    vk::PhysicalDeviceFeatures enabledCoreFeatures;

    vk::PhysicalDeviceVulkan11Features enabledFeatures11;
    vk::PhysicalDeviceVulkan12Features enabledFeatures12;
    enabledFeatures11.pNext = enabledFeatures12;
    vk::PhysicalDeviceVulkan13Features enabledFeatures13;
    enabledFeatures12.pNext = &enabledFeatures13;
    vk::PhysicalDeviceVulkan14Features enabledFeatures14;
    enabledFeatures13.pNext = &enabledFeatures14;

    vk::PhysicalDeviceDynamicRenderingFeatures enabledDynamicRenderingFeatures;
    features14.pNext = &enabledDynamicRenderingFeatures;

    enabledCoreFeatures.samplerAnisotropy = coreFeatures.features.samplerAnisotropy;
    enabledCoreFeatures.fillModeNonSolid = coreFeatures.features.fillModeNonSolid;
    enabledCoreFeatures.geometryShader = coreFeatures.features.geometryShader;
    enabledCoreFeatures.tessellationShader = coreFeatures.features.tessellationShader;
    enabledCoreFeatures.dualSrcBlend = coreFeatures.features.dualSrcBlend;

    enabledFeatures11.shaderDrawParameters = features11.shaderDrawParameters;

    enabledFeatures12.descriptorIndexing = features12.descriptorIndexing;
    enabledFeatures12.bufferDeviceAddress = features12.bufferDeviceAddress;
    enabledFeatures12.timelineSemaphore = features12.timelineSemaphore;

    enabledFeatures13.dynamicRendering = features13.dynamicRendering;
    enabledFeatures13.synchronization2 = features13.synchronization2;

    enabledFeatures14.maintenance5 = features14.maintenance5;
    enabledFeatures14.maintenance6 = features14.maintenance6;
    enabledFeatures14.pushDescriptor = features14.pushDescriptor;
    enabledFeatures14.dynamicRenderingLocalRead = features14.dynamicRenderingLocalRead;

    enabledDynamicRenderingFeatures.dynamicRendering = drFeatures.dynamicRendering;

    info.pEnabledFeatures = &enabledCoreFeatures;
    info.pNext = &enabledFeatures11;


    if (vk::ResultValue result = PhysicalDevice.createDevice(info);
        result.has_value())
    {
        Device = result.value;
    }
    else
    {
        return false;
    }

    Device.getQueue(queueInfos[0].FamilyIndex, queueInfos[0].QueueIndex, &GraphicsQueue.Queue);
    Device.getQueue(queueInfos[1].FamilyIndex, queueInfos[1].QueueIndex, &TransferQueue.Queue);

    VulkanDispatcher.init(Device);

    // Allocator - init here for access to enabled extensions
    return InitAllocator(extensions);
}

bool VulkanCore::InitAllocator(const eastl::vector<const char*>& EnabledExtensionNames)
{
    u32 allocatorFlags{0};

    eastl::string_hash_map<u32> extensionMap{};
    for (const auto* extName: EnabledExtensionNames)
    {
        extensionMap.emplace(extName, 1);
    }

    if (extensionMap.contains(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME))
        allocatorFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

    if (extensionMap.contains(VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME))
        allocatorFlags |= VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT;

    if (extensionMap.contains(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME))
        allocatorFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;

    allocatorFlags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT; // Promoted to Vulkan 1.3, but VMA documentation does not show ignored.
    allocatorFlags |= VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT; // Promoted to Vulkan 1.4, but VMA documentation does not show ignored.

    VmaAllocatorCreateInfo info{};
    info.flags = allocatorFlags;
    info.physicalDevice = PhysicalDevice;
    info.device = Device;
    info.preferredLargeHeapBlockSize = 0;
    info.pAllocationCallbacks = nullptr;
    info.pDeviceMemoryCallbacks = nullptr;
    info.pHeapSizeLimit = nullptr;

    VmaVulkanFunctions functions{};
    functions.vkGetInstanceProcAddr = VulkanDispatcher.vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr = VulkanDispatcher.vkGetDeviceProcAddr;
    info.pVulkanFunctions = &functions;

    info.instance = Instance;
    info.vulkanApiVersion = vk::ApiVersion14;

    auto res = static_cast<vk::Result>(vmaCreateAllocator(&info, &Allocator));
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kCritical, "Failed to create vulkan allocator.");
    return true;
}

bool VulkanCore::InitPipelineCache()
{
    eastl::vector<u8> pipelineCacheData = LoadPipelineCacheData();

    vk::PipelineCacheCreateInfo info;
    info.initialDataSize = pipelineCacheData.size();
    info.pInitialData = pipelineCacheData.data();
    if (const auto result = Device.createPipelineCache(info); result.has_value())
    {
        PipelineCache = result.value;
        return true;
    }
    return false;
}

void VulkanCore::Teardown()
{
    Device.waitIdle();

    Pipelines.clear();
    ShaderCache.reset();
    Swapchains.clear();

    if (Allocator)
    {
        vmaDestroyAllocator(Allocator);
    }

    if (Device)
    {
        Device.destroy();
        Device = nullptr;
    }
    PhysicalDevice = nullptr;
    if (DebugMessenger)
    {
        Instance.destroyDebugUtilsMessengerEXT(DebugMessenger);
        DebugMessenger = nullptr;
    }
    if (Instance)
    {
        Instance.destroy();
    }
}

bool VulkanCore::FindPresentQueueFamily(vk::SurfaceKHR Surface, u32& OutPresentFamily) const
{
    u32 nQueueFamilies = 0;
    PhysicalDevice.getQueueFamilyProperties2(&nQueueFamilies, nullptr);

    for (u32 i = 0; i < nQueueFamilies; ++i)
    {
        auto result = PhysicalDevice.getSurfaceSupportKHR(i, Surface);
        if (!result.has_value()) continue;
        OutPresentFamily = i;
        return true;
    }
    return false;
}

eastl::vector<VulkanCore::QueueData> VulkanCore::CreateQueueInfos(eastl::vector<vk::DeviceQueueCreateInfo>& OutQueueInfos) const
{
    u32 nQueueFamilies = 0;
    PhysicalDevice.getQueueFamilyProperties2(&nQueueFamilies, nullptr);
    eastl::vector<vk::QueueFamilyProperties2> props{nQueueFamilies, vk::QueueFamilyProperties2{}};
    PhysicalDevice.getQueueFamilyProperties2(&nQueueFamilies, props.data());

    bool graphicsFound = false;
    u32 graphicsFamilyIndex = 0;

    bool transferFound = false;
    u32 transferFamilyIndex = 0;
    bool transferUnique = false;

    for (u32 i = 0; const auto& [_1, _2, prop]: props)
    {
        if (!graphicsFound && prop.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            graphicsFound = true;
            graphicsFamilyIndex = i;
            if (!transferUnique && prop.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                transferFound = true;
                transferFamilyIndex = i;
            }
        }
        else if (!transferUnique && prop.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            transferUnique = true;
            transferFound = true;
            transferFamilyIndex = i;
        }
        ++i;
        if (graphicsFound && transferUnique) break;
    }

    if (!graphicsFound || !transferFound)
    {
        return {};
    }

    static constexpr eastl::array<f32, 2> priorities = {1.f, 1.f};

    vk::DeviceQueueCreateInfo& graphicsCreateInfo = OutQueueInfos.emplace_back();
    graphicsCreateInfo.pQueuePriorities = priorities.data();
    graphicsCreateInfo.queueFamilyIndex = graphicsFamilyIndex;
    graphicsCreateInfo.queueCount = 1;

    if (!transferUnique)
    {
        graphicsCreateInfo.queueCount += 1;
    }
    else
    {
        vk::DeviceQueueCreateInfo& transferCreateInfo = OutQueueInfos.emplace_back();
        transferCreateInfo.pQueuePriorities = priorities.data();
        transferCreateInfo.queueFamilyIndex = transferFamilyIndex;
        transferCreateInfo.queueCount = 1;
    }

    return {
        QueueData{
            .FamilyIndex = graphicsFamilyIndex,
            .QueueIndex = 0,
            .Flags = vk::QueueFlagBits::eGraphics
        },
        QueueData{
            .FamilyIndex = transferFamilyIndex,
            .QueueIndex = (transferUnique ? 0u : 1u),
            .Flags = vk::QueueFlagBits::eTransfer
        }
    };
}

bool VulkanCore::IsTransferQueueConcurrent() const
{
    return GraphicsQueue.FamilyIndex != TransferQueue.FamilyIndex;
}

eastl::vector<u8> VulkanCore::LoadPipelineCacheData() const
{
    return {}; // TODO : IO pipeline cache
}
}
