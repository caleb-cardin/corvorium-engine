// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Logging.hpp"
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
module Rendering:VulkanUtils;
import Common;
import OS;

namespace Corvorium::Rendering::Vulkan {
Log::Logger VulkanLogger{"VulkanRendering"};

auto& VulkanDispatcher = VULKAN_HPP_DEFAULT_DISPATCHER;

namespace Util {
vk::SurfaceKHR CreateWindowSurface(const vk::Instance Instance, const OS::Window* Window)
{
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(Instance, static_cast<GLFWwindow*>(Window->GetWindowRaw()), nullptr, &surface);
    CRVM_LOG_RETURN_IF(res != VK_SUCCESS, nullptr, VulkanLogger, kError, "Failed to create surface.");
    return surface;
}
}
}
