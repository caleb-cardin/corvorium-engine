// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/span.h"

#include "vma/vk_mem_alloc.h"
module Rendering:VulkanImage;
import vulkan;
import CommonTypes;
import :Types;

namespace Corvorium::Rendering::Vulkan {
class VulkanCore;

struct ImageCreateParams {
    VulkanCore* Core;
    vk::Format Format;
    vk::Extent3D Extent;
    vk::ImageUsageFlags Usage;
    eastl::span<u32> QueueFamilies;
};

class VulkanImage {
public:

    static ResultValue<VulkanImage> Create(const ImageCreateParams& Params);

    VulkanImage(const VulkanImage& Other) = delete;
    VulkanImage& operator=(const VulkanImage& Other) = delete;

    VulkanImage(VulkanImage&& Other) noexcept = default;
    VulkanImage& operator=(VulkanImage&& Other) noexcept = default;

    ~VulkanImage();

    [[nodiscard]] vk::Image GetImage() const { return Image; }
    [[nodiscard]] vk::ImageView GetImageView() const { return ImageView; }
    [[nodiscard]] VmaAllocation GetAllocation() const { return Allocation; }

    [[nodiscard]] vk::Extent3D GetExtent() const {return Extent;}
    [[nodiscard]] vk::Format GetFormat() const {return Format;}

    [[nodiscard]] vk::Result CopyFromMemory(const ByteView& Data, u64 Offset = 0);
    [[nodiscard]] vk::Result CopyToMemory(ByteView& Data, u64 Offset = 0);

private:
    Auto<VulkanCore*> Core{nullptr};
    Auto<vk::Image> Image{nullptr};
    Auto<vk::ImageView> ImageView{nullptr};
    Auto<VmaAllocation> Allocation{nullptr};
    VkDeviceSize AllocationOffset{};
    vk::Extent3D Extent{};
    vk::Format Format{};

    VulkanImage() = default;
};
}
