// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "EASTL/hash_map.h"
#include "EASTL/unique_ptr.h"

#include "GLFW/glfw3.h"
export module OS:WindowManager;
import :Window;
import :Input;
import Event;

namespace Corvorium::OS {
void WindowCloseCallback(GLFWwindow* WindowRaw);

export class CRVM_API WindowManager {
public:
    WindowManager() = delete;

    explicit WindowManager(Event::GlobalEventBus* InEventBus);

    ~WindowManager();

    WindowManager(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager& operator=(WindowManager&&) = delete;

    [[nodiscard]] bool IsInitialized() const {return Initialized;}

    static void PollEvents() { glfwPollEvents(); }

    bool IsWindowClosed(const WindowHandle Handle) { return !Windows.contains(Handle); }

    WindowHandle NewWindow(u32 Width, u32 Height, const char* Title);
    Window* GetWindow(const WindowHandle& Handle);
    void UnregisterWindow(const WindowHandle& Handle);

private:
    friend class Window;
    Event::GlobalEventBus* EventBus{nullptr};
    eastl::hash_map<WindowHandle, eastl::unique_ptr<Window>, HandleHash> Windows{};
    Input::InputState InputState{};

    bool Initialized{false};

    [[nodiscard]] Event::GlobalEventBus* GetEventHandler() const { return EventBus; }

    void UpdateKeyState(Input::Key Key, Input::KeyStateFlags StateFlags, Input::KeyModFlags ModFlags);
    void UpdateFnKeyState(Input::FnKey Key, Input::KeyStateFlags StateFlags, Input::KeyModFlags ModFlags);
    void UpdateMouseButtonState(Input::MouseButton Button, Input::KeyStateFlags StateFlags, Input::KeyModFlags ModFlags);

};

void WindowCloseCallback(GLFWwindow* WindowRaw)
{
    auto* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(WindowRaw));
    pWindow->GetManager()->UnregisterWindow(pWindow->GetHandle());
    //underlying object will be destroyed. Do not access pWindow from here.
}
}
