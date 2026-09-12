// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "GLFW/glfw3.h"
module OS:WindowCallbacksImpl;
import :WindowCallbacks;
import :WindowManager;
import :Window;
import :InputManager;
import :InputConversionHelpers;

using namespace Corvorium::OS::Internal;

namespace Corvorium::OS {

void FramebufferResizeCallback_Internal(GLFWwindow* WindowRaw, i32 Width, const i32 Height)
{
    auto* cbd = static_cast<WindowCallbackData*>(glfwGetWindowUserPointer(WindowRaw));
    cbd->pEventBus->Signal(
        WindowFramebufferResizeEvent{cbd->pWindow->GetHandle(), static_cast<u32>(Width), static_cast<u32>(Height)}
    );
}

void CloseCallback_Internal(GLFWwindow* WindowRaw)
{
    auto* cbd = static_cast<WindowCallbackData*>(glfwGetWindowUserPointer(WindowRaw));
    cbd->pEventBus->Signal(
        WindowCloseEvent{cbd->pWindow->GetHandle()}
    );

    cbd->pWindowManager->UnregisterWindow(cbd->pWindow->GetHandle());
    //underlying object will be destroyed, pWindow dangling. Do not access pWindow from this point.
    cbd->pWindow = nullptr;
}

void KeyCallback_Internal(GLFWwindow* WindowRaw, const i32 Key, [[maybe_unused]] const i32 Scancode, const i32 Action, const i32 Mods)
{
    auto* cbd = static_cast<WindowCallbackData*>(glfwGetWindowUserPointer(WindowRaw));
    if (IsKey(Key))
    {
        cbd->pInputManager->UpdateKeyState(FromGLFWKey(Key), FromGLFWState(Action), FromGLFWMods(Mods));
    }
    else if (IsFnKey(Key))
    {
        cbd->pInputManager->UpdateFnKeyState(FromGLFWFnKey(Key), FromGLFWState(Action), FromGLFWMods(Mods));
    }
}

void MouseButtonCallback_Internal(GLFWwindow* WindowRaw, const i32 Button, const i32 Action, const i32 Mods)
{
    auto* cbd = static_cast<WindowCallbackData*>(glfwGetWindowUserPointer(WindowRaw));
    cbd->pInputManager->UpdateMouseButtonState(FromGLFWMouseButton(Button), FromGLFWState(Action), FromGLFWMods(Mods));
}

void WindowFocusedCallback_Internal(GLFWwindow* WindowRaw, const i32 Focused)
{
    auto* cbd = static_cast<WindowCallbackData*>(glfwGetWindowUserPointer(WindowRaw));
    cbd->pInputManager->SetFocusedWindow(Focused == GLFW_TRUE ? cbd->pWindow->GetHandle() : WindowHandle{});
}

void CursorPosCallback_Internal(GLFWwindow* WindowRaw, const f64 Xpos, const f64 Ypos)
{
    auto* cbd = static_cast<WindowCallbackData*>(glfwGetWindowUserPointer(WindowRaw));
    cbd->pInputManager->SetCursorPosition(Xpos, Ypos);
}
}
