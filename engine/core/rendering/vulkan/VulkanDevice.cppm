// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Logging.hpp"
#include "EASTL/vector.h"
#include "EASTL/array.h"
#include "EASTL/hash_map.h"

#include "vulkan/vulkan.hpp"
module Rendering:VkDevice;
import :Device;
import :VkContext;
import :VkSwapchain;
import :VulkanUtils;

namespace Corvorium::Rendering::Vulkan {
class VulkanDevice : public RenderDevice {
    struct QueueData {
        u32 FamilyIndex{};
        u32 QueueIndex{};
        vk::Queue Queue{};
        vk::QueueFlags Flags{};
    };

public:
    VulkanDevice() = delete;
    explicit VulkanDevice(VulkanContext* ParentContext);
    ~VulkanDevice() override = default;

    bool Initialize() override;
    void Shutdown() override;

    bool CreateSwapchainForWindow(OS::Window* InWindow);

private:
    VulkanContext* Context;

    vk::PhysicalDevice PhysicalDevice;
    vk::Device Device;

    QueueData GraphicsQueue;
    QueueData TransferQueue;

    eastl::hash_map<OS::WindowHandle, VulkanSwapchain> Swapchains;

    vk::PipelineCache PipelineCache;

    void SelectPhysicalDevice();
    void InitDevice();
    void InitPipelineCache();

    eastl::vector<QueueData> CreateQueueInfos(eastl::vector<vk::DeviceQueueCreateInfo>& OutQueueInfos);
    [[nodiscard]] bool IsTransferQueueConcurrent() const;

    [[nodiscard]] eastl::vector<u8> LoadPipelineCacheData() const;
};

VulkanDevice::VulkanDevice(VulkanContext* ParentContext)
    : Context{ParentContext}
{}

bool VulkanDevice::Initialize()
{
    SelectPhysicalDevice();
    if (!PhysicalDevice)
    {
        return false;
    }

    InitDevice();

    return true;
}

void VulkanDevice::Shutdown()
{
    Device.destroy();
    PhysicalDevice = nullptr;
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

    const u32 queueFamilyIndices[] = {GraphicsQueue.FamilyIndex, TransferQueue.FamilyIndex};
    const eastl::span<const u32> indices(queueFamilyIndices);
    VulkanSwapchainCreateParams params{
        .Instance = Context->Instance,
        .PhysicalDevice = PhysicalDevice,
        .Device = Device,
        .Window = InWindow,
        .QueueFamilyIndices = IsTransferQueueConcurrent()
                                  ? eastl::span<const u32>(queueFamilyIndices)
                                  : eastl::span<const u32>(queueFamilyIndices).subspan(0, 1)
    };

    auto [_, success] = Swapchains.try_emplace(InWindow->GetHandle(), params);
    return success;
}

void VulkanDevice::SelectPhysicalDevice()
{
    u32 deviceCount = 0;
    vk::Result res = Context->Instance.enumeratePhysicalDevices(&deviceCount, nullptr);
    if (deviceCount == 0)
    {
        // TODO : Print results
        return;
    }
    eastl::vector<vk::PhysicalDevice> physicalDevices;
    physicalDevices.resize(deviceCount);
    res = Context->Instance.enumeratePhysicalDevices(&deviceCount, physicalDevices.data());

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
            deviceScores[i] += 500;
        }
        else if (props.properties.deviceType == vk::PhysicalDeviceType::eCpu)
        {
            deviceScores[i] += 100;
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
}

void VulkanDevice::InitDevice()
{
    eastl::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    auto queueInfos = CreateQueueInfos(queueCreateInfos);
    GraphicsQueue = queueInfos[0];
    TransferQueue = queueInfos[1];

    vk::DeviceCreateInfo info;
    info.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
    info.pQueueCreateInfos = queueCreateInfos.data();

    // Extensions

    constexpr eastl::array<const char*, 1> reqDeviceExtNames = {
        "KHR_swapchain"
    };
    constexpr const eastl::array<const char*, 8> optDeviceExtNames = {
        "VK_EXT_memory_budget",
        "VK_EXT_memory_priority",
        "VK_AMD_device_coherent_memory",
        "VK_EXT_descriptor_heap",
        "VK_KHR_maintenance7",
        "VK_KHR_acceleration_structure",
        "VK_KHR_ray_tracing_pipeline",
        "VK_EXT_mesh_shader"
    };

    u32 propertyCount = 0;
    vk::Result res = PhysicalDevice.enumerateDeviceExtensionProperties(nullptr, &propertyCount, nullptr);
    eastl::vector<vk::ExtensionProperties> properties(propertyCount, vk::ExtensionProperties{});
    res = PhysicalDevice.enumerateDeviceExtensionProperties(nullptr, &propertyCount, properties.data());
    // TODO check results

    auto checkOptExtName = [optDeviceExtNames](const eastl::vector<vk::ExtensionProperties>& available) -> eastl::vector<const char*> {
        eastl::vector<const char*> exts;
        for (const auto& prop: available)
        {
            for (const char* checkName: optDeviceExtNames)
            {
                if (std::strcmp(prop.extensionName, checkName) == 0)
                {
                    exts.emplace_back(checkName);
                }
            }
        }
        return exts;
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


    if (vk::ResultValue result = PhysicalDevice.createDevice(info); result.has_value())
    {
        Device = result.value;
    }
    else
    {
        // TODO : Log errors
    }

    Device.getQueue(queueInfos[0].FamilyIndex, queueInfos[0].QueueIndex, &GraphicsQueue.Queue);
    Device.getQueue(queueInfos[1].FamilyIndex, queueInfos[1].QueueIndex, &TransferQueue.Queue);
}

void VulkanDevice::InitPipelineCache()
{
    eastl::vector<u8> pipelineCacheData = LoadPipelineCacheData();

    vk::PipelineCacheCreateInfo info;
    info.initialDataSize = pipelineCacheData.size();
    info.pInitialData = pipelineCacheData.data();
    if (const auto result = Device.createPipelineCache(info); result.has_value())
    {
        PipelineCache = result.value;
    }
    else
    {
        PipelineCache = nullptr;
    }
}

eastl::vector<VulkanDevice::QueueData> VulkanDevice::CreateQueueInfos(eastl::vector<vk::DeviceQueueCreateInfo>& OutQueueInfos)
{
    u32 nQueueFamilies = 0;
    PhysicalDevice.getQueueFamilyProperties2(&nQueueFamilies, nullptr);
    eastl::vector<vk::QueueFamilyProperties2> props{nQueueFamilies, vk::QueueFamilyProperties2{}};
    PhysicalDevice.getQueueFamilyProperties2(&nQueueFamilies, props.data());

    // TODO : Queue creation

    bool graphicsFound = false;
    u32 graphicsFamilyIndex = 0;
    bool transferFound = false;
    u32 transferFamilyIndex = 0;
    bool transferUnique = false;
    for (u32 i = 0; const auto& [_1, _2, prop]: props)
    {
        if (prop.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            graphicsFound = true;
            graphicsFamilyIndex = i;
            if (!transferUnique && prop.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                transferFound = true;
                transferFamilyIndex = i;
            }
        }
        if (!transferUnique && prop.queueFlags & vk::QueueFlagBits::eTransfer
            && !(prop.queueFlags & vk::QueueFlagBits::eGraphics))
        {
            transferUnique = true;
            transferFound = true;
            transferFamilyIndex = i;
        }
    }

    if (!graphicsFound || !transferFound)
    {
        return {};
    }

    constexpr float priorities[] = {-1.f, -1.f};

    vk::DeviceQueueCreateInfo& graphicsCreateInfo = OutQueueInfos.emplace_back();
    graphicsCreateInfo.pQueuePriorities = priorities;
    graphicsCreateInfo.queueFamilyIndex = graphicsFamilyIndex;
    graphicsCreateInfo.queueCount = 1;

    if (!transferUnique)
    {
        graphicsCreateInfo.queueCount += 1;
    }
    else
    {
        vk::DeviceQueueCreateInfo& transferCreateInfo = OutQueueInfos.emplace_back();
        transferCreateInfo.pQueuePriorities = priorities;
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
