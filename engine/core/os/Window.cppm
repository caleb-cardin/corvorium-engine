// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "GLFW/glfw3.h"
#include "EASTL/atomic.h"
export module OS:Window;
import Common;

namespace Corvorium::OS {
class WindowManager;

export struct CRVM_API WindowCreateParams {
    u32 Width = 1600;
    u32 Height = 900;
    const char* Title = "Default Window";
};

namespace Internal {
using WindowCloseCallback = GLFWwindowclosefun;
}

export class CRVM_API Window {
public:
    using Handle_t = HandleType<Window>;

    Window() = delete;

    Window(u32 Width, u32 Height, const char* Title)
        : Handle{Handle_t::New()}
        , pWindow{nullptr}
    {
        if (glfwInit())
        {
            gCounter.fetch_add(1, eastl::memory_order_relaxed);
            pWindow = glfwCreateWindow(static_cast<i32>(Width), static_cast<i32>(Height), Title, nullptr, nullptr);
        }
    }

    explicit Window(const WindowCreateParams& Params)
        : Handle{Handle_t::New()}
        , pWindow{nullptr}
    {
        if (glfwInit())
        {
            gCounter.fetch_add(1, eastl::memory_order_relaxed);
            pWindow = glfwCreateWindow(static_cast<i32>(Params.Width), static_cast<i32>(Params.Height), Params.Title, nullptr, nullptr);
        }
    }

    ~Window()
    {
        glfwDestroyWindow(pWindow);
        if (gCounter.fetch_sub(1, eastl::memory_order_relaxed) == 1)
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
    friend class WindowManager;
    WindowManager* Manager{nullptr};
#include "SilenceDllMemberWarningBegin.inc"
    inline static eastl::atomic<u64> gCounter{0};
    Handle_t Handle{};
#include "SilenceDllMemberWarningEnd.inc"
    GLFWwindow* pWindow;

    void SetWindowCloseCallbackFn(Internal::WindowCloseCallback Fn)
    {
        glfwSetWindowUserPointer(pWindow, this);

        glfwSetWindowCloseCallback(pWindow, Fn);
    }
};

export using WindowHandle = Window::Handle_t;
}
