// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "EASTL/hash_map.h"
#include "EASTL/unique_ptr.h"

#include "GLFW/glfw3.h"
export module OS:WindowManager;
import :Window;
import :WindowCallbacks;
import Event;

namespace Corvorium::OS {
class InputManager;

export class CRVM_API WindowManager {
public:
    WindowManager() = delete;

    WindowManager(InputManager* InputManager, Event::GlobalEventBus* EventBus);

    ~WindowManager();

    WindowManager(const WindowManager&) = delete;
    WindowManager(WindowManager&&) = default;
    WindowManager& operator=(const WindowManager&) = delete;
    WindowManager& operator=(WindowManager&&) = default;

    [[nodiscard]] bool IsInitialized() const {return Initialized;}

    static void PollEvents() { glfwPollEvents(); }

    bool IsWindowClosed(const WindowHandle Handle) { return !Windows.contains(Handle); }

    WindowHandle NewWindow(u32 Width, u32 Height, const char* Title);
    Window* GetWindow(const WindowHandle& Handle);
    void UnregisterWindow(const WindowHandle& Handle);

private:
    friend class Window;
    WindowCallbackData TemplateCallbackData{};
    eastl::hash_map<WindowHandle, eastl::unique_ptr<Window>, HandleHash> Windows{};

    bool Initialized{false};
};
}
