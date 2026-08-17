// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/span.h"
#include "EASTL/vector.h"
#include "vulkan/vulkan.hpp"
module Rendering:VkSwapchain;
import OS;
import :VulkanUtils;

namespace Corvorium::Rendering::Vulkan {
struct VulkanSwapchainCreateParams {
    vk::Instance Instance;
    vk::PhysicalDevice PhysicalDevice;
    vk::Device Device;
    OS::Window* Window;
    eastl::span<const u32> QueueFamilyIndices;
};

class VulkanSwapchain {
public:
    VulkanSwapchain() = delete;
    explicit VulkanSwapchain(const VulkanSwapchainCreateParams& Params);

private:
    vk::SwapchainKHR Swapchain;
    vk::SurfaceKHR Surface;
    eastl::vector<vk::Image> Images;
    eastl::vector<vk::ImageView> ImageViews;
};


static vk::SurfaceFormat2KHR SelectOptimalFormat(const eastl::vector<vk::SurfaceFormat2KHR>& Formats)
{
    for (const auto& format : Formats)
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
    for (const auto& mode : Modes)
    {
        if (mode == vk::PresentModeKHR::eMailbox) return mode;
    }
    return vk::PresentModeKHR::eFifo;
}

VulkanSwapchain::VulkanSwapchain(const VulkanSwapchainCreateParams& Params)
{
    Surface = Util::CreateWindowSurface(Params.Instance, Params.Window);

    vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo;
    surfaceInfo.surface = Surface;
    vk::SurfaceCapabilities2KHR surfaceCaps;
    vk::Result res = Params.PhysicalDevice.getSurfaceCapabilities2KHR(&surfaceInfo, &surfaceCaps);
    //TODO : check result

    u32 formatCount = 0;
    res = Params.PhysicalDevice.getSurfaceFormats2KHR(&surfaceInfo, &formatCount, nullptr);
    //TODO : check result
    eastl::vector<vk::SurfaceFormat2KHR> surfaceFormats{formatCount, vk::SurfaceFormat2KHR{}};
    res = Params.PhysicalDevice.getSurfaceFormats2KHR(&surfaceInfo, &formatCount, surfaceFormats.data());

    const vk::SurfaceFormat2KHR selectedFormat = SelectOptimalFormat(surfaceFormats);

    u32 presentModeCount = 0;
    res = Params.PhysicalDevice.getSurfacePresentModesKHR(Surface, &presentModeCount, nullptr);
    //TODO : check result
    eastl::vector<vk::PresentModeKHR> surfacePresentModes{presentModeCount, vk::PresentModeKHR{}};
    res = Params.PhysicalDevice.getSurfacePresentModesKHR(Surface, &presentModeCount, surfacePresentModes.data());

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
    info.queueFamilyIndexCount = Params.QueueFamilyIndices.size();
    info.pQueueFamilyIndices = Params.QueueFamilyIndices.data();
    info.preTransform = surfaceCaps.surfaceCapabilities.currentTransform;
    info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    info.presentMode = selectedPresentMode;
    info.clipped = true;
    info.oldSwapchain = nullptr;


    if (const vk::ResultValue scResult = Params.Device.createSwapchainKHR(info); scResult.has_value())
    {
        Swapchain = scResult.value;
    }
    else
    {
        // TODO : Error
    }

    u32 imageCount;
    res = Params.Device.getSwapchainImagesKHR(Swapchain, &imageCount, nullptr);
    Images.resize(imageCount);
    res = Params.Device.getSwapchainImagesKHR(Swapchain, &imageCount, Images.data());

    ImageViews.resize(imageCount);
    for (u32 i = 0; const auto& image: Images)
    {
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = selectedFormat.surfaceFormat.format;
        viewInfo.components = vk::ComponentMapping{};
        viewInfo.subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0u,
            .levelCount = 1u,
            .baseArrayLayer = 0u,
            .layerCount = 1u
        };

        if (auto viewResult = Params.Device.createImageView(viewInfo); viewResult.has_value())
        {
            ImageViews[i] = viewResult.value;
        }
    }
}
}
