// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "glfw/glfw3.h"
module OS:WindowImpl;
import :Window;
import :WindowManager;
import :Input;
import :WindowCallbacks;

namespace Corvorium::OS {

Window::Window(const WindowCallbackData& InCallbackData, u32 Width, u32 Height, const char* Title)
    : Handle{Handle_t::New()}
    , pWindow{nullptr}
{
    CallbackData = InCallbackData;
    CallbackData.pWindow = this;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    pWindow = glfwCreateWindow(static_cast<i32>(Width), static_cast<i32>(Height), Title, nullptr, nullptr);

    glfwSetWindowUserPointer(pWindow, &CallbackData);
    glfwSetFramebufferSizeCallback(pWindow, FramebufferResizeCallback_Internal);
    glfwSetWindowCloseCallback(pWindow, CloseCallback_Internal);
    glfwSetKeyCallback(pWindow, KeyCallback_Internal);
    glfwSetMouseButtonCallback(pWindow, MouseButtonCallback_Internal);
    glfwSetCursorPosCallback(pWindow, CursorPosCallback_Internal);
}
}
