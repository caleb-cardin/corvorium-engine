// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "GLFW/glfw3.h"

#include "EASTL/hash_map.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/vector_map.h"

export module OS:WindowManager;
import :Window;

namespace Corvorium::OS {
void WindowCloseCallback(GLFWwindow* WindowRaw);

export class WindowManager {
public:
    WindowManager() = default;
    ~WindowManager();

    WindowHandle NewWindow(const WindowCreateParams& Params)
    {
        auto window = eastl::make_unique<Window>(Params);
        auto [it, success] = Windows.try_emplace(window->GetHandle(), eastl::move(window));
        if (success)
        {
            it->second->SetWindowCloseCallbackFn(WindowCloseCallback);
            return it->first;
        }
        return {};
    }

    Window* GetWindow(const WindowHandle Handle)
    {
        auto it = Windows.find(Handle);
        if (it != Windows.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    void UnregisterWindow(const WindowHandle Handle)
    {
        Windows.erase(Handle);
    }

private:
    eastl::hash_map<WindowHandle, eastl::unique_ptr<Window>> Windows;
};

void WindowCloseCallback(GLFWwindow* WindowRaw)
{
    auto* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
    pWindow->GetManager()->UnregisterWindow(pWindow->GetHandle());
    //underlying object will be destroyed. Do not access pWindow from here.
}
}
