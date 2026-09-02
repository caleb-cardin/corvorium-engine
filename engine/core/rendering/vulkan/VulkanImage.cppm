// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/span.h"
#include "vulkan/vulkan.hpp"
#include "vma/vk_mem_alloc.h"
module Rendering:VulkanImage;
import Common;
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
    VulkanImage() = delete;

    static ResultValue<VulkanImage> Create(const ImageCreateParams& Params);

    VulkanImage(const VulkanImage& Other) = delete;
    VulkanImage& operator=(const VulkanImage& Other) = delete;

    VulkanImage(VulkanImage&& Other) noexcept;
    VulkanImage& operator=(VulkanImage&& Other) noexcept;

    ~VulkanImage();

    [[nodiscard]] vk::Image GetImage() const {return Image;}
    [[nodiscard]] vk::ImageView GetImageView() const {return ImageView;}
    [[nodiscard]] VmaAllocation GetAllocation() const {return Allocation;}

private:
    VulkanCore* Core{nullptr};
    vk::Image Image{nullptr};
    vk::ImageView ImageView{nullptr};
    VmaAllocation Allocation{nullptr};

    VulkanImage(VulkanCore* Core, vk::Image Image, vk::ImageView View, VmaAllocation Alloc);
};
}
