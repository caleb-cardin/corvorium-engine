// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/Assert.hpp"
#include "event/EventType.hpp"

#include "GLFW/glfw3.h"
export module OS:Window;
import TypeDefs;
import CommonTypes;
import Event;

namespace Corvorium::OS {
class WindowManager;
export class Window;

export struct WindowFramebufferResizeEvent;
export struct WindowCloseEvent;

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

    [[nodiscard]] WindowManager* GetManager() const { return Manager; }
    [[nodiscard]] Handle_t GetHandle() const { return Handle; }
    [[nodiscard]] void* GetWindowRaw() const { return pWindow; }


    /**
     * @return true if size is valid. false if window is minimized.
     */
    [[nodiscard]] void GetFramebufferSize(u32& Width, u32& Height) const
    {
        i32 iWidth, iHeight;
        glfwGetFramebufferSize(pWindow, &iWidth, &iHeight);
        Width = static_cast<u32>(iWidth);
        Height = static_cast<u32>(iHeight);
    }

private:
    friend class WindowManager;
    WindowManager* Manager{nullptr};
    Handle_t Handle{};
    GLFWwindow* pWindow{nullptr};

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
        CRVM_ASSERT(Manager);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        pWindow = glfwCreateWindow(static_cast<i32>(Width), static_cast<i32>(Height), Title, nullptr, nullptr);
        glfwSetWindowUserPointer(pWindow, this);
        glfwSetFramebufferSizeCallback(pWindow, Window::FramebufferResizeCallback_Internal);
        glfwSetKeyCallback(pWindow, Window::KeyCallback_Internal);
    }

    static void FramebufferResizeCallback_Internal(GLFWwindow* WindowRaw, i32 Width, i32 Height);
    static void CloseCallback_Internal(GLFWwindow* WindowRaw);
    static void KeyCallback_Internal(GLFWwindow* WindowRaw, i32 Key, i32 Scancode, i32 Action, i32 Mods);

    void SetWindowCloseCallbackFn(Internal::WindowCloseCallback Fn) { glfwSetWindowCloseCallback(pWindow, Fn); }
};

export using WindowHandle = Window::Handle_t;

struct WindowFramebufferResizeEvent {
    DECLARE_EVENT_TYPE_TRAITS(Corvorium::OS::WindowFramebufferResizeEvent);
    WindowHandle Window;
    u32 Width{};
    u32 Height{};
};

struct WindowCloseEvent {
    DECLARE_EVENT_TYPE_TRAITS(Corvorium::OS::WindowCloseEvent);
    WindowHandle Window;
};
}
