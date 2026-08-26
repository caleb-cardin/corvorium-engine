// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "EASTL/hash_map.h"
#include "EASTL/variant.h"
#include "EASTL/unique_ptr.h"

#include "vulkan/vulkan.hpp"
module Rendering:VkDeviceImpl;
import :VkDevice;
import :VkCommandBuffer;
import :CommandBufferImpl;

namespace Corvorium::Rendering::Vulkan {
VulkanDevice::VulkanDevice(vk::Instance Instance, OS::Window* MainWindow)
    : Instance{Instance}
{
    if (!SelectPhysicalDevice())
    {
        Teardown();
        return;
    }

    vk::SurfaceKHR MainSurface = Util::CreateWindowSurface(Instance, MainWindow);
    bool PresentFound = FindPresentQueueFamily(MainSurface, PresentQueueFamily);
    if (!PresentFound)
    {
        Instance.destroySurfaceKHR(MainSurface);
        return;
    }

    auto [scIter, success] = Swapchains.try_emplace(MainWindow->GetHandle(), VulkanSwapchain::CreateUninitializedSwapchain(MainSurface));
    if (!success)
    {
        Instance.destroySurfaceKHR(MainSurface);
        return;
    }

    if (!InitDevice())
    {
        Teardown();
        return;
    }

    bool swapchainInitialized = scIter->second.Initialize({
        .Instance = Instance,
        .Device = this,
        .Window = MainWindow,
        .GraphicsQueue = GraphicsQueue.FamilyIndex,
        .PresentQueue = PresentQueueFamily
    });
    if (!swapchainInitialized)
    {
        Teardown();
        return;
    }

    Initialized = true;
}

VulkanDevice::~VulkanDevice()
{
    Teardown();
}


bool VulkanDevice::IsInitialized() const
{
    return Initialized;
}

RenderError VulkanDevice::BeginFrame(OS::WindowHandle Window)
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

    res = swapchain.InitializedCommandList();
    CRVM_ASSERT(res == vk::Result::eSuccess);
    return RenderError::Success;
}

ResultValue<CommandBuffer> VulkanDevice::AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority)
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

    return CommandBuffer{eastl::make_unique<CommandBufferImpl>(VulkanCommandBuffer{this, cb})};
}

RenderError VulkanDevice::EndFrame(OS::WindowHandle Window)
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

// TODO : This function has multiple fail states, consider proper error returns.
/**
 * CreateSwapchainForWindow
 * @param InWindow Window handle to create the swapchain surface from
 * @return true if swapchain was created, false if swapchain already exists for the given window
 */
bool VulkanDevice::CreateSwapchainForWindow(OS::Window* InWindow)
{
    if (Swapchains.contains(InWindow->GetHandle()))
    {
        // TODO : Error, disallow multiple SC for singular window
        return false;
    }

    VulkanSwapchainCreateParams params{
        .Instance = Instance,
        .Device = this,
        .Window = InWindow,
        .GraphicsQueue = GraphicsQueue.FamilyIndex,
        .PresentQueue = PresentQueueFamily
    };

    auto [_, success] = Swapchains.try_emplace(InWindow->GetHandle(), VulkanSwapchain::CreateInitializedSwapchain(params));
    if (!success)
    {
        return false;
    }

    return true;
}

void VulkanDevice::OnWindowClosed(OS::Window* Window, void* OwningDevice)
{
    CRVM_ASSERT(Window);
    CRVM_ASSERT(OwningDevice);
    auto* Device = static_cast<VulkanDevice*>(OwningDevice);
    const OS::WindowHandle handle = Window->GetHandle();
    if (Device->Swapchains.contains(handle))
    {
        Device->Swapchains.erase(handle);
    }
}

vk::Pipeline VulkanDevice::FindPipeline(PipelineHandle Pipeline)
{
    auto it = Pipelines.find(Pipeline);
    if (it != Pipelines.end())
    {
        return it->second.Get();
    }
    return nullptr;
}

bool VulkanDevice::SelectPhysicalDevice()
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

bool VulkanDevice::InitDevice()
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

    constexpr eastl::array<const char*, 1> reqDeviceExtNames = {
        "VK_KHR_swapchain"
    };
    constexpr const eastl::array<const char*, 9> optDeviceExtNames = {
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
    vk::PhysicalDeviceVulkan12Features features12;
    vk::PhysicalDeviceVulkan13Features features13;
    vk::PhysicalDeviceVulkan14Features features14;
    vk::PhysicalDeviceDynamicRenderingFeatures drFeatures;
    coreFeatures.pNext = &features12;
    features12.pNext = &features13;
    features13.pNext = &features14;
    features14.pNext = &drFeatures;

    PhysicalDevice.getFeatures2(&coreFeatures);
    vk::PhysicalDeviceFeatures enabledCoreFeatures;

    vk::PhysicalDeviceVulkan12Features enabledFeatures12;
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
    info.pNext = &enabledFeatures12;


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

    return true;
}

bool VulkanDevice::InitPipelineCache()
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

void VulkanDevice::Teardown()
{
    Swapchains.clear();
    if (Device)
    {
        Device.destroy();
        Device = nullptr;
    }
    PhysicalDevice = nullptr;
}

bool VulkanDevice::FindPresentQueueFamily(vk::SurfaceKHR Surface, u32& OutPresentFamily) const
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

eastl::vector<VulkanDevice::QueueData> VulkanDevice::CreateQueueInfos(eastl::vector<vk::DeviceQueueCreateInfo>& OutQueueInfos) const
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

bool VulkanDevice::IsTransferQueueConcurrent() const
{
    return GraphicsQueue.FamilyIndex != TransferQueue.FamilyIndex;
}

eastl::vector<u8> VulkanDevice::LoadPipelineCacheData() const
{
    return {}; // TODO : IO pipeline cache
}
}
