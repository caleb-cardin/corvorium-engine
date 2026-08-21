// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "GLFW/glfw3.h"
export module OS:Window;
import Common;

namespace Corvorium::OS {
class WindowManager;
export class Window;

using WindowFramebufferResizeCallbackFn = void(*)(u32 Width, u32 Height, void* pData);
using WindowCloseCallbackFn = void(*)(Window* Window, void* pData);

export struct WindowCallbackData {
    WindowFramebufferResizeCallbackFn ResizeFn{nullptr};
    void* ResizeUserPtr{nullptr};
    WindowCloseCallbackFn CloseFn{nullptr};
    void* CloseUserPtr{nullptr};
};

namespace Internal {
using WindowCloseCallback = GLFWwindowclosefun;
}

struct WindowCreateParams {
    WindowManager* Manager{nullptr};
    u32 Width = 1600;
    u32 Height = 900;
    const char* Title = "Default Window";
};

class CRVM_API Window {
public:
    using Handle_t = HandleType<Window>;

    Window() = delete;

    ~Window()
    {
        glfwDestroyWindow(pWindow);
    }

    [[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(pWindow); }
    static void PollEvents() { glfwPollEvents(); }

    [[nodiscard]] WindowManager* GetManager() const { return Manager; }
    [[nodiscard]] Handle_t GetHandle() const { return Handle; }
    [[nodiscard]] void* GetWindowRaw() const { return pWindow; }

    void SetCallbackData(const WindowCallbackData& InCallbackData) { CallbackData = InCallbackData; }

private:
    friend class WindowManager;
    WindowManager* Manager{nullptr};
    Handle_t Handle{};
    GLFWwindow* pWindow{nullptr};

    WindowCallbackData CallbackData{};

    Window(WindowManager* InManager, u32 Width, u32 Height, const char* Title)
        : Manager{InManager}
        , Handle{Handle_t::New()}
        , pWindow{nullptr} { SetupWindow(Width, Height, Title); }

    explicit Window(const WindowCreateParams& Params)
        : Manager{Params.Manager}
        , Handle{Handle_t::New()}
        , pWindow{nullptr} { SetupWindow(Params.Width, Params.Height, Params.Title); }

    void SetupWindow(u32 Width, u32 Height, const char* Title)
    {
        Assert(Manager);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        pWindow = glfwCreateWindow(static_cast<i32>(Width), static_cast<i32>(Height), Title, nullptr, nullptr);
        glfwSetWindowUserPointer(pWindow, this);
        glfwSetFramebufferSizeCallback(pWindow, Window::FramebufferResizeCallback_Internal);
    }

    static void FramebufferResizeCallback_Internal(GLFWwindow* WindowRaw, i32 Width, i32 Height)
    {
        auto* Win = static_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
        Win->CallbackData.ResizeFn(static_cast<u32>(Width), static_cast<u32>(Height), Win->CallbackData.ResizeUserPtr);
    }

    static void CloseCallback_Internal(GLFWwindow* WindowRaw)
    {
        auto* Win = static_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
        Win->CallbackData.CloseFn(Win, Win->CallbackData.CloseUserPtr);
    }

    void SetWindowCloseCallbackFn(Internal::WindowCloseCallback Fn) { glfwSetWindowCloseCallback(pWindow, Fn); }
};

export using WindowHandle = Window::Handle_t;
}
