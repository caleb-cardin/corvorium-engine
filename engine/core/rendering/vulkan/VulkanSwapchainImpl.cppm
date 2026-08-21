// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Logging.hpp"
#include "EASTL/vector.h"
#include "vulkan/vulkan.hpp"
module Rendering:VkSwapchainImpl;
import :VkSwapchain;
import :VkDevice;

namespace Corvorium::Rendering::Vulkan {
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

VulkanSwapchain::VulkanSwapchain(const VulkanSwapchainCreateParams& Params)
    : Device{Params.Device}
{
    Assert(Device);
    Assert(Params.Window);
    WindowHandle = Params.Window->GetHandle();
    Surface = Util::CreateWindowSurface(Params.Instance, Params.Window);

    OS::WindowCallbackData CallbackData{
        .ResizeFn = OnWindowResized,
        .ResizeUserPtr = this,
        .CloseFn = VulkanDevice::OnWindowClosed,
        .CloseUserPtr = Device
    };
    Params.Window->SetCallbackData(CallbackData);

    CreateOrRecreateSwapchain(Params);
}

VulkanSwapchain::~VulkanSwapchain()
{
    vk::Device vulkanDevice = Device->GetDevice();

    for (auto view: ImageViews)
    {
        vulkanDevice.destroyImageView(view);
    }
    ImageViews.clear();
    Images.clear();

    vulkanDevice.destroySwapchainKHR(Swapchain);

    vk::Instance vulkanInstance = Device->GetInstance();
    vulkanInstance.destroySurfaceKHR(Surface);
}

bool VulkanSwapchain::CreateOrRecreateSwapchain(const VulkanSwapchainCreateParams& Params)
{
    vk::Device vkDevice = Device->GetDevice();
    if (Swapchain)
    {
        Images.clear();
        for (auto view: ImageViews)
        {
            vkDevice.destroyImageView(view);
            CRVM_LOG(VulkanLogger, kInfo, "ImageView destroyed");
        }
        ImageViews.clear();
    }

    vk::PhysicalDevice vkPhysicalDevice = Device->GetPhysicalDevice();

    vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo;
    surfaceInfo.surface = Surface;
    vk::SurfaceCapabilities2KHR surfaceCaps;
    vk::Result res = vkPhysicalDevice.getSurfaceCapabilities2KHR(&surfaceInfo, &surfaceCaps, VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface capabilities.");

    u32 formatCount = 0;
    res = vkPhysicalDevice.getSurfaceFormats2KHR(&surfaceInfo, &formatCount, nullptr, VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface formats.");
    eastl::vector<vk::SurfaceFormat2KHR> surfaceFormats{formatCount, vk::SurfaceFormat2KHR{}};
    res = vkPhysicalDevice.getSurfaceFormats2KHR(&surfaceInfo, &formatCount, surfaceFormats.data(), VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface formats.");

    const vk::SurfaceFormat2KHR selectedFormat = SelectOptimalFormat(surfaceFormats);

    u32 presentModeCount = 0;
    res = vkPhysicalDevice.getSurfacePresentModesKHR(Surface, &presentModeCount, nullptr, VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface present modes.");
    eastl::vector<vk::PresentModeKHR> surfacePresentModes{presentModeCount, vk::PresentModeKHR{}};
    res = vkPhysicalDevice.getSurfacePresentModesKHR(Surface, &presentModeCount, surfacePresentModes.data(), VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, false, VulkanLogger, kError, "Failed to get surface present modes.");

    const vk::PresentModeKHR selectedPresentMode = SelectOptimalPresentMode(surfacePresentModes);

    vk::SwapchainCreateInfoKHR info;
    info.surface = Surface;
    info.minImageCount = std::min(surfaceCaps.surfaceCapabilities.minImageCount + 1, surfaceCaps.surfaceCapabilities.maxImageCount);
    info.imageFormat = selectedFormat.surfaceFormat.format;
    info.imageColorSpace = selectedFormat.surfaceFormat.colorSpace;
    info.imageExtent = surfaceCaps.surfaceCapabilities.currentExtent;
    info.imageArrayLayers = 1;
    info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    info.imageSharingMode = Params.QueueFamilyIndices.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
    info.queueFamilyIndexCount = static_cast<u32>(Params.QueueFamilyIndices.size());
    info.pQueueFamilyIndices = Params.QueueFamilyIndices.data();
    info.preTransform = surfaceCaps.surfaceCapabilities.currentTransform;
    info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    info.presentMode = selectedPresentMode;
    info.clipped = true;
    info.oldSwapchain = Swapchain;

    const vk::ResultValue scResult = vkDevice.createSwapchainKHR(info);
    CRVM_LOG_RETURN_IF(!scResult.has_value(), false, VulkanLogger, kError, "Failed to create swapchain.");
    Swapchain = scResult.value;

    u32 imageCount;
    res = vkDevice.getSwapchainImagesKHR(Swapchain, &imageCount, nullptr);
    Images.resize(imageCount);
    res = vkDevice.getSwapchainImagesKHR(Swapchain, &imageCount, Images.data());

    ImageViews.resize(imageCount);
    for (u32 i = 0; const auto& image: Images)
    {
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = selectedFormat.surfaceFormat.format;
        viewInfo.components = vk::ComponentMapping{};
        viewInfo.subresourceRange = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

        if (auto viewResult = vkDevice.createImageView(viewInfo); viewResult.has_value())
        {
            ImageViews[i] = viewResult.value;
        }
        ++i;
    }
}

void VulkanSwapchain::OnWindowResized(u32 Width, u32 Height, void* Swapchain)
{}
}
