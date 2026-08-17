// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "GLFW/glfw3.h"
#include <atomic>
export module OS:Window;
import Common;
namespace Corvorium::OS {

export struct WindowCreateParams {
    u32 Width = 1600;
    u32 Height = 900;
    const char* Title = "Default Window";
};

export class Window {
public:
    using Handle = u64;

    Window() = delete;
    Window(u32 Width, u32 Height, const char* Title);
    explicit Window(const WindowCreateParams& Params);
    ~Window();

    [[nodiscard]] void* GetWindowRaw() const;

private:
    inline static std::atomic_uint32_t gCounter{0};
    GLFWwindow* pWindow;
};
export using WindowHandle = Window::Handle;

Window::Window(const u32 Width, const u32 Height, const char* Title)
    : pWindow{nullptr}
{
    if (glfwInit())
    {
        gCounter.fetch_add(1, std::memory_order_relaxed);
        pWindow = glfwCreateWindow(Width, Height, Title, nullptr, nullptr);
    }
}

Window::Window(const WindowCreateParams& Params)
    : pWindow{nullptr}
{
    if (glfwInit())
    {
        gCounter.fetch_add(1, std::memory_order_relaxed);
        pWindow = glfwCreateWindow(Params.Width, Params.Height, Params.Title, nullptr, nullptr);
    }
}

Window::~Window()
{
    glfwDestroyWindow(pWindow);
    if (gCounter.fetch_sub(1, std::memory_order_relaxed) == 1)
    {
        glfwTerminate();
    }
}

void* Window::GetWindowRaw() const
{
    return pWindow;
}
}
