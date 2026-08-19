// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "GLFW/glfw3.h"
#include <atomic>
export module OS:Window;
import Common;

namespace Corvorium::OS {
class WindowManager;

export struct WindowCreateParams {
    u32 Width = 1600;
    u32 Height = 900;
    const char* Title = "Default Window";
};
namespace Internal {
using WindowCloseCallback = GLFWwindowclosefun;
}

export class Window {
public:
    using Handle_t = HandleType<Window>;

    Window() = delete;

    Window(u32 Width, u32 Height, const char* Title)
    : Handle{Handle_t::New()}
    , pWindow{nullptr}
    {
        if (glfwInit())
        {
            gCounter.fetch_add(1, std::memory_order_relaxed);
            pWindow = glfwCreateWindow(static_cast<i32>(Width), static_cast<i32>(Height), Title, nullptr, nullptr);
        }
    }

    explicit Window(const WindowCreateParams& Params)
    : Handle{Handle_t::New()}
    , pWindow{nullptr}
    {
        if (glfwInit())
        {
            gCounter.fetch_add(1, std::memory_order_relaxed);
            pWindow = glfwCreateWindow(static_cast<i32>(Params.Width), static_cast<i32>(Params.Height), Params.Title, nullptr, nullptr);
        }
    }

    ~Window()
    {
        glfwDestroyWindow(pWindow);
        if (gCounter.fetch_sub(1, std::memory_order_relaxed) == 1)
        {
            glfwTerminate();
        }
    }


    [[nodiscard]] WindowManager* GetManager() const
    {
        return Manager;
    }
    [[nodiscard]] Handle_t GetHandle() const
    {
        return Handle;
    }
    [[nodiscard]] void* GetWindowRaw() const
    {
        return pWindow;
    }

private:
    inline static std::atomic_uint32_t gCounter{0};
    friend class WindowManager;
    WindowManager* Manager{nullptr};
    Handle_t Handle{};
    GLFWwindow* pWindow;

    void SetWindowCloseCallbackFn(Internal::WindowCloseCallback Fn)
    {
        glfwSetWindowUserPointer(pWindow, this);

        glfwSetWindowCloseCallback(pWindow, Fn);
    }
};

export using WindowHandle = Window::Handle_t;

}
