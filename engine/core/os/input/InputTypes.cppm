// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/FlagTypeMacro.hpp"
#include "GLFW/glfw3.h"

export module OS:InputTypes;
import Flags;

namespace Corvorium {
export namespace OS {
enum class KeyStateFlagBits : u8 {
    None = 0,
    Pressed FLAG_BIT(0),
    Released FLAG_BIT(1),
    Repeating FLAG_BIT(2),
    JustUpdated FLAG_BIT(3),

    ActiveMask = Pressed | Released
};

enum class Key : u8 {
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Space,
    Comma,
    Period,
    Slash,
    Semicolon,
    Apostrophe,
    LeftBracket,
    RightBracket,
    Backslash,
    Tilde,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Minus,
    Equal,

    World1,
    World2,
    COUNT
};

enum class FnKey : u8 {
    Escape,
    Tab,
    CapsLock,
    Backspace,
    Enter,

    Insert,
    Delete,
    Home,
    End,
    PageUp,
    PageDown,

    Up,
    Down,
    Right,
    Left,

    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,

    PrintScreen,
    ScrollLock,
    Pause,

    NumLock,
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    NumpadDecimal,
    NumpadDivide,
    NumpadMultiply,
    NumpadSubtract,
    NumpadAdd,
    NumpadEnter,
    NumpadEqual,

    RightShift,
    LeftShift,
    RightControl,
    LeftControl,
    RightAlt,
    LeftAlt,
    RightSuper,
    LeftSuper,

    Menu,
    COUNT
};

enum class KeyModFlagBits : u8 {
    None,
    Shift FLAG_BIT(0),
    Ctrl FLAG_BIT(1),
    Alt FLAG_BIT(2),
    Super FLAG_BIT(3),
    CapsLock FLAG_BIT(4),
    NumLock FLAG_BIT(5),
};

enum class MouseButton : u8 {
    Mb1,
    Mb2,
    Mb3,
    Mb4,
    Mb5,
    Mb6,
    Mb7,
    Mb8,
    COUNT,

    Lmb = Mb1,
    Rmb = Mb2,
    Mmb = Mb3,
};

enum class GamepadButton : u8 {
    FaceUp,
    FaceDown,
    FaceLeft,
    FaceRight,
    LeftBumper,
    RightBumper,
    Back,
    Start,
    Guide,
    LeftThumb,
    RightThumb,
    DpadUp,
    DpadDown,
    DpadRight,
    DpadLeft,
    COUNT,

    XB_A = FaceDown,
    XB_B = FaceRight,
    XB_X = FaceLeft,
    XB_Y = FaceUp,

    PS_X = FaceDown,
    PS_Circle = FaceRight,
    PS_Square = FaceLeft,
    PS_Triangle = FaceUp
};

enum class GamepadAxis : u8 {
    LeftX,
    LeftY,
    RightX,
    RightY,
    LeftTrigger,
    RightTrigger,
    COUNT,
};
}

DECLARE_FLAG_TYPE_NS(KeyStateFlags, OS::KeyStateFlagBits, OS)
DECLARE_FLAG_TYPE_NS(KeyModFlags, OS::KeyModFlagBits, OS)

namespace OS {

inline bool WasUpdated(KeyStateFlags OldState, KeyStateFlags NewState)
{
    return (OldState & KeyStateFlagBits::ActiveMask) ^ (NewState & KeyStateFlagBits::ActiveMask);
}

}
}


