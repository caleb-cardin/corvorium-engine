// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "vulkan/vulkan.hpp"
#include "vma/vk_mem_alloc.h"
module Rendering:VulkanImageImpl;
import :VulkanImage;
import :VulkanCore;

namespace Corvorium::Rendering::Vulkan {
VmaAllocationCreateFlags GetOptimalCreateFlags(vk::ImageUsageFlags Usage)
{
    VmaAllocationCreateFlags outFlags{0};
    if (Usage & vk::ImageUsageFlagBits::eColorAttachment || Usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
    {
        outFlags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
    }
    return outFlags;
}

VmaMemoryUsage GetOptimalUsage(vk::ImageUsageFlags Usage)
{
    VmaMemoryUsage outUsage{VMA_MEMORY_USAGE_AUTO};
    if (Usage & vk::ImageUsageFlagBits::eColorAttachment || Usage & vk::ImageUsageFlagBits::eDepthStencilAttachment)
    {
        outUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }
    return outUsage;
}

ResultValue<VulkanImage> VulkanImage::Create(const ImageCreateParams& Params)
{
    CRVM_ASSERT(Params.Core);

    vk::ImageCreateInfo imageInfo;
    // TODO : Image creation params
    imageInfo.imageType = vk::ImageType::e2D;
    imageInfo.format = Params.Format;
    imageInfo.extent = Params.Extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = vk::SampleCountFlagBits::e1;
    imageInfo.tiling = vk::ImageTiling::eOptimal;
    imageInfo.usage = Params.Usage;
    imageInfo.sharingMode = Params.QueueFamilies.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
    imageInfo.queueFamilyIndexCount = static_cast<u32>(Params.QueueFamilies.size());
    imageInfo.pQueueFamilyIndices = Params.QueueFamilies.data();
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.flags = GetOptimalCreateFlags(Params.Usage);
    allocCreateInfo.usage = GetOptimalUsage(Params.Usage);
    allocCreateInfo.requiredFlags = 0;
    allocCreateInfo.preferredFlags = 0;
    allocCreateInfo.memoryTypeBits = 0;
    allocCreateInfo.pool = nullptr;
    allocCreateInfo.pUserData = nullptr;
    allocCreateInfo.priority = 0.f;
    allocCreateInfo.minAlignment = 0;

    vk::Device device = Params.Core->GetDevice();
    VmaAllocator allocator = Params.Core->GetAllocator();

    vk::Image image;
    VmaAllocation allocation;
    [[maybe_unused]] VmaAllocationInfo allocInfo; {
        VkResult res = vmaCreateImage(allocator,
                                      reinterpret_cast<VkImageCreateInfo*>(&imageInfo),
                                      &allocCreateInfo,
                                      reinterpret_cast<VkImage*>(&image),
                                      &allocation,
                                      &allocInfo);
        CRVM_LOG_RETURN_IF(res != VK_SUCCESS,
                           ResultError(RenderError::InitializationFailed),
                           VulkanLogger,
                           kError,
                           "Failed to create Image.")
    }

    vk::ImageViewCreateInfo viewInfo;
    viewInfo.image = image;
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = Params.Format;
    viewInfo.components = vk::ComponentMapping{};

    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    viewInfo.subresourceRange.baseMipLevel = 0u;
    viewInfo.subresourceRange.levelCount = 1u;
    viewInfo.subresourceRange.baseArrayLayer = 0u;
    viewInfo.subresourceRange.layerCount = 1u;


    vk::ImageView view; {
        const auto result = device.createImageView(viewInfo);
        if (!result.has_value())
        {
            CRVM_LOG(VulkanLogger, kError, "Failed to create Image.");
            vmaDestroyImage(allocator, image, allocation);
            return ResultError(RenderError::InitializationFailed);
        }
        view = result.value;
    }

    return VulkanImage(Params.Core, image, view, allocation);
}

VulkanImage::VulkanImage(VulkanImage&& Other) noexcept
    : Core{Other.Core}
    , Image{Other.Image}
    , ImageView{Other.ImageView}
    , Allocation{Other.Allocation}
{
    Other.Core = nullptr;
    Other.Image = nullptr;
    Other.ImageView = nullptr;
    Other.Allocation = nullptr;
}

VulkanImage& VulkanImage::operator=(VulkanImage&& Other) noexcept
{
    Core = Other.Core;
    Image = Other.Image;
    ImageView = Other.ImageView;
    Allocation = Other.Allocation;

    Other.Core = nullptr;
    Other.Image = nullptr;
    Other.ImageView = nullptr;
    Other.Allocation = nullptr;
    return *this;
}

VulkanImage::~VulkanImage()
{
    if (Core)
    {
        vk::Device device = Core->GetDevice();
        if (ImageView) device.destroyImageView(ImageView);

        VmaAllocator allocator = Core->GetAllocator();
        vmaDestroyImage(allocator, Image, Allocation);
    }
}

VulkanImage::VulkanImage(VulkanCore* Core, vk::Image Image, vk::ImageView View, VmaAllocation Alloc)
    : Core{Core}
    , Image{Image}
    , ImageView{View}
    , Allocation{Alloc}
{}
}
