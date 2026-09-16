// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "vma/vk_mem_alloc.h"
module Rendering:VulkanImageImpl;
import vulkan;
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

VmaMemoryUsage GetOptimalUsage(const vk::ImageUsageFlags Usage)
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
    imageInfo.imageType = Params.Extent.depth == 1 ? vk::ImageType::e2D : vk::ImageType::e3D;
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
    VmaAllocationInfo info; {
        VkResult res = vmaCreateImage(allocator,
                                      reinterpret_cast<VkImageCreateInfo*>(&imageInfo),
                                      &allocCreateInfo,
                                      reinterpret_cast<VkImage*>(&image),
                                      &allocation,
                                      &info);
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

    VulkanImage newImage{};
    newImage.Core = Params.Core;
    newImage.Image = image;
    newImage.ImageView = view;
    newImage.Allocation = allocation;
    newImage.AllocationOffset = info.offset;
    newImage.Extent = Params.Extent;
    newImage.Format = Params.Format;
    return newImage;
}

VulkanImage::~VulkanImage()
{
    if (Core)
    {
        vk::Device device = Core->GetDevice();
        if (ImageView) device.destroyImageView(ImageView);

        VmaAllocator allocator = Core->GetAllocator();
        vmaDestroyImage(allocator, Image.Native(), Allocation);
    }
}

vk::Result VulkanImage::CopyFromMemory(const ByteView& Data, const u64 Offset)
{
    const auto imgBufferSize = Util::GetExtentTotal(Extent);
    const vk::DeviceSize memSize = Data.size_bytes();
    CRVM_LOG_RETURN_IF((imgBufferSize - Offset) < memSize,
                       vk::Result::eErrorUnknown,
                       VulkanLogger,
                       kError,
                       "Attempted to fill image outside image bounds.");
    return vk::Result{vmaCopyMemoryToAllocation(Core->GetAllocator(), Data.data(), Allocation, AllocationOffset + Offset, memSize)};
}

vk::Result VulkanImage::CopyToMemory(ByteView& Data, const u64 Offset)
{
    const auto imgBufferSize = Util::GetExtentTotal(Extent);
    CRVM_LOG_RETURN_IF((imgBufferSize - Offset) < Data.size_bytes(),
                       vk::Result::eErrorUnknown,
                       VulkanLogger,
                       kError,
                       "Attempted to read image outside image bounds.");
    return vk::Result{vmaCopyAllocationToMemory(Core->GetAllocator(), Allocation, AllocationOffset + Offset, (void*)Data.data(), Data.size_bytes())};
}
}
