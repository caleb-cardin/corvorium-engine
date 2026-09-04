// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "glfw/glfw3.h"
module OS:WindowImpl;
import :Window;
import :WindowManager;

namespace Corvorium::OS {
void Window::FramebufferResizeCallback_Internal(GLFWwindow* WindowRaw, i32 Width, i32 Height)
{
    auto* Win = static_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
    Win->Manager->GetEventHandler()->Signal(
        WindowFramebufferResizeEvent{Win->Handle, static_cast<u32>(Width), static_cast<u32>(Height)}
    );
}

void Window::CloseCallback_Internal(GLFWwindow* WindowRaw)
{
    auto* Win = static_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
    Win->Manager->GetEventHandler()->Signal(
        WindowCloseEvent{Win->Handle}
    );
}
}
