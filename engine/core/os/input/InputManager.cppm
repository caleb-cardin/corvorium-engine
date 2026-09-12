// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

export module OS:InputManager;
import :Window;
import :Input;

namespace Corvorium::OS {

class InputManager {
public:


    void SetFocusedWindow(WindowHandle Handle) {FocusedWindow = Handle;}
    void SetCursorPosition(const f64 Xpos, const f64 Ypos) {CursorPos = {Xpos, Ypos};}

    void UpdateKeyState(Key Key, KeyStateFlags StateFlags, KeyModFlags ModFlags);
    void UpdateFnKeyState(FnKey Key, KeyStateFlags StateFlags, KeyModFlags ModFlags);
    void UpdateMouseButtonState(MouseButton Button, KeyStateFlags StateFlags, KeyModFlags ModFlags);

    static void KeyCallback_Internal(GLFWwindow* WindowRaw, i32 Key, i32 Scancode, i32 Action, i32 Mods);
    static void MouseButtonCallback_Internal(GLFWwindow* WindowRaw, i32 Button, i32 Action, i32 Mods);
    static void WindowFocusedCallback_Internal(GLFWwindow* WindowRaw, i32 Focused);
    static void CursorPosCallback_Internal(GLFWwindow* WindowRaw, f64 Xpos, f64 Ypos);

private:
    OS::WindowHandle FocusedWindow{};
    CursorPosition CursorPos{};
    InputState State{};
};

}

