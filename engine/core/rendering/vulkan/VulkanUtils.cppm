// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
module Rendering:VulkanUtils;
import OS;
namespace Corvorium::Rendering::Vulkan::Util {

vk::SurfaceKHR CreateWindowSurface(const vk::Instance Instance, const OS::Window* Window)
{
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(Instance, static_cast<GLFWwindow*>(Window->GetWindowRaw()), nullptr, &surface);
    // TODO : check return value
    return surface;
}

}
