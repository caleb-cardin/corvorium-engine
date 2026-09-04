// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/Assert.hpp"

#include "GLFW/glfw3.h"
#include "EASTL/hash_map.h"
#include "EASTL/unique_ptr.h"

export module OS:WindowManager;
import :Window;
import Event;

namespace Corvorium::OS {
void WindowCloseCallback(GLFWwindow* WindowRaw);

export class CRVM_API WindowManager {
public:
    WindowManager() = delete;

    WindowManager(Event::GlobalEventBus* InEventBus)
        : EventBus{InEventBus}
        , Windows{}
    {
        CRVM_ASSERT(EventBus);
        if (glfwInit())
        {
            Initialized = true;
        }
    }

    ~WindowManager()
    {
        Windows.clear();
        glfwTerminate();
    }

    WindowManager(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager& operator=(WindowManager&&) = delete;

    [[nodiscard]] bool IsInitialized() const
    {
        return Initialized;
    }

    bool IsWindowClosed(const WindowHandle Handle) { return !Windows.contains(Handle); }

    WindowHandle NewWindow(u32 Width, u32 Height, const char* Title)
    {
        WindowCreateParams Params{.Manager = this, .Width = Width, .Height = Height, .Title = Title};
        auto window = eastl::unique_ptr<Window>(new Window(Params));
        auto [it, success] = Windows.try_emplace(window->GetHandle(), eastl::move(window));
        if (success)
        {
            it->second->SetWindowCloseCallbackFn(WindowCloseCallback);
            return it->first;
        }
        return {};
    }

    Window* GetWindow(const WindowHandle& Handle)
    {
        auto it = Windows.find(Handle);
        if (it != Windows.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    void UnregisterWindow(const WindowHandle& Handle)
    {
        Windows.erase(Handle);
    }

    [[nodiscard]] Event::GlobalEventBus* GetEventHandler() const { return EventBus; }

private:
    Event::GlobalEventBus* EventBus{nullptr};
    eastl::hash_map<WindowHandle, eastl::unique_ptr<Window>, HandleHash> Windows{};

    bool Initialized{false};
};

void WindowCloseCallback(GLFWwindow* WindowRaw)
{
    auto* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
    pWindow->GetManager()->UnregisterWindow(pWindow->GetHandle());
    //underlying object will be destroyed. Do not access pWindow from here.
}
}
