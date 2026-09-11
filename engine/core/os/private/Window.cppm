// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "glfw/glfw3.h"
module OS:WindowImpl;
import :Window;
import :WindowManager;
import :InputConversionHelpers;

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

void Window::KeyCallback_Internal(GLFWwindow* WindowRaw, i32 Key, [[maybe_unused]] i32 Scancode, i32 Action, i32 Mods)
{
    auto* Win = static_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
    if (Input::Internal::IsKey(Key))
    {
        Win->Manager->UpdateKeyState(Input::Internal::FromGLFWKey(Key), Input::Internal::FromGLFWState(Action), Input::Internal::FromGLFWMods(Mods));
    }
    else if (Input::Internal::IsFnKey(Key))
    {
        Win->Manager->UpdateFnKeyState(Input::Internal::FromGLFWFnKey(Key), Input::Internal::FromGLFWState(Action), Input::Internal::FromGLFWMods(Mods));
    }
}
}
