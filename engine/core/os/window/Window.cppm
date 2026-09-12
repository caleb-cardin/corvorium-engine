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
import :WindowCallbacks;

namespace Corvorium::Input {
class InputManager;
}

namespace Corvorium::OS {

export struct WindowFramebufferResizeEvent;
export struct WindowCloseEvent;

export class CRVM_API Window {
public:
    using Handle_t = HandleType<Window>;

    Window() = delete;

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(Window&&) = default;

    ~Window()
    {
        glfwDestroyWindow(pWindow);
    }

    [[nodiscard]] bool ShouldClose() const { return glfwWindowShouldClose(pWindow); }

    [[nodiscard]] Handle_t GetHandle() const { return Handle; }
    [[nodiscard]] void* GetWindowRaw() const { return pWindow; }

    [[nodiscard]] void GetFramebufferSize(u32& Width, u32& Height) const
    {
        i32 iWidth, iHeight;
        glfwGetFramebufferSize(pWindow, &iWidth, &iHeight);
        Width = static_cast<u32>(iWidth);
        Height = static_cast<u32>(iHeight);
    }

private:
    friend class WindowManager;
    Handle_t Handle{};
    Auto<GLFWwindow*> pWindow{nullptr};
    WindowCallbackData CallbackData{};

    Window(const WindowCallbackData& InCallbackData, u32 Width, u32 Height, const char* Title);

    void SetupWindow(u32 Width, u32 Height, const char* Title);
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
