// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "GLFW/glfw3.h"
export module OS:WindowCallbacks;
import TypeDefs;
import Event;

export namespace Corvorium::OS {
class Window;
class WindowManager;
class InputManager;
}

namespace Corvorium::OS {
struct WindowCallbackData {
    Window* pWindow{nullptr};
    WindowManager* pWindowManager{nullptr};
    InputManager* pInputManager{nullptr};
    Event::GlobalEventBus* pEventBus{nullptr};
};

void FramebufferResizeCallback_Internal(GLFWwindow* WindowRaw, i32 Width, const i32 Height);
void CloseCallback_Internal(GLFWwindow* WindowRaw);

void KeyCallback_Internal(GLFWwindow* WindowRaw, i32 Key, i32 Scancode, i32 Action, i32 Mods);
void MouseButtonCallback_Internal(GLFWwindow* WindowRaw, i32 Button, i32 Action, i32 Mods);
void WindowFocusedCallback_Internal(GLFWwindow* WindowRaw, i32 Focused);
void CursorPosCallback_Internal(GLFWwindow* WindowRaw, f64 Xpos, f64 Ypos);

}
