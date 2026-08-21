// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/span.h"
#include "EASTL/vector.h"
#include "vulkan/vulkan.hpp"
module Rendering:VkSwapchain;
import OS;
import Common;
import :VulkanUtils;

namespace Corvorium::Rendering::Vulkan {
class VulkanDevice;

struct VulkanSwapchainCreateParams {
    vk::Instance Instance;
    VulkanDevice* Device;
    OS::Window* Window;
    eastl::span<const u32> QueueFamilyIndices;
};

class VulkanSwapchain {
public:
    VulkanSwapchain() = delete;
    explicit VulkanSwapchain(const VulkanSwapchainCreateParams& Params);
    ~VulkanSwapchain();

    vk::SwapchainKHR GetSwapchain() const { return Swapchain; }
    vk::SurfaceKHR GetSurface() const { return Surface; }

    bool CreateOrRecreateSwapchain(const VulkanSwapchainCreateParams& Params);

    static void OnWindowResized(u32 Width, u32 Height, void* Swapchain);

private:
    VulkanDevice* Device{nullptr};
    OS::WindowHandle WindowHandle{};

    vk::SwapchainKHR Swapchain{nullptr};
    vk::SurfaceKHR Surface{nullptr};
    eastl::vector<vk::Image> Images{};
    eastl::vector<vk::ImageView> ImageViews{};
};
}
