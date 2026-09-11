// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "GLFW/glfw3.h"
module OS:InputConversionHelpers;
import :InputTypes;

namespace Corvorium::Input::Internal {

static_assert(static_cast<u8>(KeyModFlagBits::Shift) == static_cast<u8>(GLFW_MOD_SHIFT));
static_assert(static_cast<u8>(KeyModFlagBits::Ctrl) == static_cast<u8>(GLFW_MOD_CONTROL));
static_assert(static_cast<u8>(KeyModFlagBits::Alt) == static_cast<u8>(GLFW_MOD_ALT));
static_assert(static_cast<u8>(KeyModFlagBits::Super) == static_cast<u8>(GLFW_MOD_SUPER));
static_assert(static_cast<u8>(KeyModFlagBits::CapsLock) == static_cast<u8>(GLFW_MOD_CAPS_LOCK));
static_assert(static_cast<u8>(KeyModFlagBits::NumLock) == static_cast<u8>(GLFW_MOD_NUM_LOCK));

using GLFWKeyType = decltype(GLFW_KEY_SPACE);

bool IsKey(GLFWKeyType RawKey)
{
    return RawKey >= GLFW_KEY_SPACE && RawKey <= GLFW_KEY_WORLD_2;
}

bool IsFnKey(GLFWKeyType RawKey)
{
    return RawKey >= GLFW_KEY_ESCAPE && RawKey <= GLFW_KEY_MENU;
}

GLFWKeyType ToGLFWKey(Key Key)
{
    switch (Key)
    {
    case Key::A: return GLFW_KEY_A;
    case Key::B: return GLFW_KEY_B;
    case Key::C: return GLFW_KEY_C;
    case Key::D: return GLFW_KEY_D;
    case Key::E: return GLFW_KEY_E;
    case Key::F: return GLFW_KEY_F;
    case Key::G: return GLFW_KEY_G;
    case Key::H: return GLFW_KEY_H;
    case Key::I: return GLFW_KEY_I;
    case Key::J: return GLFW_KEY_J;
    case Key::K: return GLFW_KEY_K;
    case Key::L: return GLFW_KEY_L;
    case Key::M: return GLFW_KEY_M;
    case Key::N: return GLFW_KEY_N;
    case Key::O: return GLFW_KEY_O;
    case Key::P: return GLFW_KEY_P;
    case Key::Q: return GLFW_KEY_Q;
    case Key::R: return GLFW_KEY_R;
    case Key::S: return GLFW_KEY_S;
    case Key::T: return GLFW_KEY_T;
    case Key::U: return GLFW_KEY_U;
    case Key::V: return GLFW_KEY_V;
    case Key::W: return GLFW_KEY_W;
    case Key::X: return GLFW_KEY_X;
    case Key::Y: return GLFW_KEY_Y;
    case Key::Z: return GLFW_KEY_Z;
    case Key::Space: return GLFW_KEY_SPACE;
    case Key::Comma: return GLFW_KEY_COMMA;
    case Key::Period: return GLFW_KEY_PERIOD;
    case Key::Slash: return GLFW_KEY_SLASH;
    case Key::Semicolon: return GLFW_KEY_SEMICOLON;
    case Key::Apostrophe: return GLFW_KEY_APOSTROPHE;
    case Key::LeftBracket: return GLFW_KEY_LEFT_BRACKET;
    case Key::RightBracket: return GLFW_KEY_RIGHT_BRACKET;
    case Key::Backslash: return GLFW_KEY_BACKSLASH;
    case Key::Tilde: return GLFW_KEY_GRAVE_ACCENT;
    case Key::Num0: return GLFW_KEY_0;
    case Key::Num1: return GLFW_KEY_1;
    case Key::Num2: return GLFW_KEY_2;
    case Key::Num3: return GLFW_KEY_3;
    case Key::Num4: return GLFW_KEY_4;
    case Key::Num5: return GLFW_KEY_5;
    case Key::Num6: return GLFW_KEY_6;
    case Key::Num7: return GLFW_KEY_7;
    case Key::Num8: return GLFW_KEY_8;
    case Key::Num9: return GLFW_KEY_9;
    case Key::Minus: return GLFW_KEY_MINUS;
    case Key::Equal: return GLFW_KEY_EQUAL;
    case Key::World1: return GLFW_KEY_WORLD_1;
    case Key::World2: return GLFW_KEY_WORLD_2;
    default:
    {
        CRVM_ASSERT_MSG(false, "Unhandled key type!");
        return 0;
    }
    }
}

Key FromGLFWKey(GLFWKeyType Key)
{
    switch (Key)
    {
    case GLFW_KEY_A: return Key::A;
    case GLFW_KEY_B: return Key::B;
    case GLFW_KEY_C: return Key::C;
    case GLFW_KEY_D: return Key::D;
    case GLFW_KEY_E: return Key::E;
    case GLFW_KEY_F: return Key::F;
    case GLFW_KEY_G: return Key::G;
    case GLFW_KEY_H: return Key::H;
    case GLFW_KEY_I: return Key::I;
    case GLFW_KEY_J: return Key::J;
    case GLFW_KEY_K: return Key::K;
    case GLFW_KEY_L: return Key::L;
    case GLFW_KEY_M: return Key::M;
    case GLFW_KEY_N: return Key::N;
    case GLFW_KEY_O: return Key::O;
    case GLFW_KEY_P: return Key::P;
    case GLFW_KEY_Q: return Key::Q;
    case GLFW_KEY_R: return Key::R;
    case GLFW_KEY_S: return Key::S;
    case GLFW_KEY_T: return Key::T;
    case GLFW_KEY_U: return Key::U;
    case GLFW_KEY_V: return Key::V;
    case GLFW_KEY_W: return Key::W;
    case GLFW_KEY_X: return Key::X;
    case GLFW_KEY_Y: return Key::Y;
    case GLFW_KEY_Z: return Key::Z;
    case GLFW_KEY_SPACE: return Key::Space;
    case GLFW_KEY_COMMA: return Key::Comma;
    case GLFW_KEY_PERIOD: return Key::Period;
    case GLFW_KEY_SLASH: return Key::Slash;
    case GLFW_KEY_SEMICOLON: return Key::Semicolon;
    case GLFW_KEY_APOSTROPHE: return Key::Apostrophe;
    case GLFW_KEY_LEFT_BRACKET: return Key::LeftBracket;
    case GLFW_KEY_RIGHT_BRACKET: return Key::RightBracket;
    case GLFW_KEY_BACKSLASH: return Key::Backslash;
    case GLFW_KEY_GRAVE_ACCENT: return Key::Tilde;
    case GLFW_KEY_0: return Key::Num0;
    case GLFW_KEY_1: return Key::Num1;
    case GLFW_KEY_2: return Key::Num2;
    case GLFW_KEY_3: return Key::Num3;
    case GLFW_KEY_4: return Key::Num4;
    case GLFW_KEY_5: return Key::Num5;
    case GLFW_KEY_6: return Key::Num6;
    case GLFW_KEY_7: return Key::Num7;
    case GLFW_KEY_8: return Key::Num8;
    case GLFW_KEY_9: return Key::Num9;
    case GLFW_KEY_MINUS: return Key::Minus;
    case GLFW_KEY_EQUAL: return Key::Equal;
    case GLFW_KEY_WORLD_1: return Key::World1;
    case GLFW_KEY_WORLD_2: return Key::World2;
    default:
    {
        CRVM_ASSERT_MSG(false, "Unhandled key type!");
        return Key::COUNT;
    }
    }
}


GLFWKeyType ToGLFWFnKey(FnKey Key)
{
    switch (Key) {
    case FnKey::Escape: return GLFW_KEY_ESCAPE;
    case FnKey::Tab: return GLFW_KEY_TAB;
    case FnKey::CapsLock: return GLFW_KEY_CAPS_LOCK;
    case FnKey::Backspace: return GLFW_KEY_BACKSPACE;
    case FnKey::Enter: return GLFW_KEY_ENTER;
    case FnKey::Insert: return GLFW_KEY_INSERT;
    case FnKey::Delete: return GLFW_KEY_DELETE;
    case FnKey::Home: return GLFW_KEY_HOME;
    case FnKey::End: return GLFW_KEY_END;
    case FnKey::PageUp: return GLFW_KEY_PAGE_UP;
    case FnKey::PageDown: return GLFW_KEY_PAGE_DOWN;
    case FnKey::Up: return GLFW_KEY_UP;
    case FnKey::Down: return GLFW_KEY_DOWN;
    case FnKey::Right: return GLFW_KEY_RIGHT;
    case FnKey::Left: return GLFW_KEY_LEFT;
    case FnKey::F1: return GLFW_KEY_F1;
    case FnKey::F2: return GLFW_KEY_F2;
    case FnKey::F3: return GLFW_KEY_F3;
    case FnKey::F4: return GLFW_KEY_F4;
    case FnKey::F5: return GLFW_KEY_F5;
    case FnKey::F6: return GLFW_KEY_F6;
    case FnKey::F7: return GLFW_KEY_F7;
    case FnKey::F8: return GLFW_KEY_F8;
    case FnKey::F9: return GLFW_KEY_F9;
    case FnKey::F10: return GLFW_KEY_F10;
    case FnKey::F11: return GLFW_KEY_F11;
    case FnKey::F12: return GLFW_KEY_F12;
    case FnKey::F13: return GLFW_KEY_F13;
    case FnKey::F14: return GLFW_KEY_F14;
    case FnKey::F15: return GLFW_KEY_F15;
    case FnKey::F16: return GLFW_KEY_F16;
    case FnKey::F17: return GLFW_KEY_F17;
    case FnKey::F18: return GLFW_KEY_F18;
    case FnKey::F19: return GLFW_KEY_F19;
    case FnKey::F20: return GLFW_KEY_F20;
    case FnKey::F21: return GLFW_KEY_F21;
    case FnKey::F22: return GLFW_KEY_F22;
    case FnKey::F23: return GLFW_KEY_F23;
    case FnKey::F24: return GLFW_KEY_F24;
    case FnKey::F25: return GLFW_KEY_F25;
    case FnKey::PrintScreen: return GLFW_KEY_PRINT_SCREEN;
    case FnKey::ScrollLock: return GLFW_KEY_SCROLL_LOCK;
    case FnKey::Pause: return GLFW_KEY_PAUSE;
    case FnKey::NumLock: return GLFW_KEY_NUM_LOCK;
    case FnKey::Numpad0: return GLFW_KEY_KP_0;
    case FnKey::Numpad1: return GLFW_KEY_KP_1;
    case FnKey::Numpad2: return GLFW_KEY_KP_2;
    case FnKey::Numpad3: return GLFW_KEY_KP_3;
    case FnKey::Numpad4: return GLFW_KEY_KP_4;
    case FnKey::Numpad5: return GLFW_KEY_KP_5;
    case FnKey::Numpad6: return GLFW_KEY_KP_6;
    case FnKey::Numpad7: return GLFW_KEY_KP_7;
    case FnKey::Numpad8: return GLFW_KEY_KP_8;
    case FnKey::Numpad9: return GLFW_KEY_KP_9;
    case FnKey::NumpadDecimal: return GLFW_KEY_KP_DECIMAL;
    case FnKey::NumpadDivide: return GLFW_KEY_KP_DIVIDE;
    case FnKey::NumpadMultiply: return GLFW_KEY_KP_MULTIPLY;
    case FnKey::NumpadSubtract: return GLFW_KEY_KP_SUBTRACT;
    case FnKey::NumpadAdd: return GLFW_KEY_KP_ADD;
    case FnKey::NumpadEnter: return GLFW_KEY_KP_ENTER;
    case FnKey::NumpadEqual: return GLFW_KEY_KP_EQUAL;
    case FnKey::RightShift: return GLFW_KEY_RIGHT_SHIFT;
    case FnKey::LeftShift: return GLFW_KEY_LEFT_SHIFT;
    case FnKey::RightControl: return GLFW_KEY_RIGHT_CONTROL;
    case FnKey::LeftControl: return GLFW_KEY_LEFT_CONTROL;
    case FnKey::RightAlt: return GLFW_KEY_RIGHT_ALT;
    case FnKey::LeftAlt: return GLFW_KEY_LEFT_ALT;
    case FnKey::RightSuper: return GLFW_KEY_RIGHT_SUPER;
    case FnKey::LeftSuper: return GLFW_KEY_LEFT_SUPER;
    case FnKey::Menu: return GLFW_KEY_MENU;
    default:
    {
        CRVM_ASSERT_MSG(false, "Unhandled key type!");
        return 0;
    }
    }
}

FnKey FromGLFWFnKey(GLFWKeyType Key)
{
    switch (Key) {
    case GLFW_KEY_ESCAPE: return FnKey::Escape;
    case GLFW_KEY_TAB: return FnKey::Tab;
    case GLFW_KEY_CAPS_LOCK: return FnKey::CapsLock;
    case GLFW_KEY_BACKSPACE: return FnKey::Backspace;
    case GLFW_KEY_ENTER: return FnKey::Enter;
    case GLFW_KEY_INSERT: return FnKey::Insert;
    case GLFW_KEY_DELETE: return FnKey::Delete;
    case GLFW_KEY_HOME: return FnKey::Home;
    case GLFW_KEY_END: return FnKey::End;
    case GLFW_KEY_PAGE_UP: return FnKey::PageUp;
    case GLFW_KEY_PAGE_DOWN: return FnKey::PageDown;
    case GLFW_KEY_UP: return FnKey::Up;
    case GLFW_KEY_DOWN: return FnKey::Down;
    case GLFW_KEY_RIGHT: return FnKey::Right;
    case GLFW_KEY_LEFT: return FnKey::Left;
    case GLFW_KEY_F1: return FnKey::F1;
    case GLFW_KEY_F2: return FnKey::F2;
    case GLFW_KEY_F3: return FnKey::F3;
    case GLFW_KEY_F4: return FnKey::F4;
    case GLFW_KEY_F5: return FnKey::F5;
    case GLFW_KEY_F6: return FnKey::F6;
    case GLFW_KEY_F7: return FnKey::F7;
    case GLFW_KEY_F8: return FnKey::F8;
    case GLFW_KEY_F9: return FnKey::F9;
    case GLFW_KEY_F10: return FnKey::F10;
    case GLFW_KEY_F11: return FnKey::F11;
    case GLFW_KEY_F12: return FnKey::F12;
    case GLFW_KEY_F13: return FnKey::F13;
    case GLFW_KEY_F14: return FnKey::F14;
    case GLFW_KEY_F15: return FnKey::F15;
    case GLFW_KEY_F16: return FnKey::F16;
    case GLFW_KEY_F17: return FnKey::F17;
    case GLFW_KEY_F18: return FnKey::F18;
    case GLFW_KEY_F19: return FnKey::F19;
    case GLFW_KEY_F20: return FnKey::F20;
    case GLFW_KEY_F21: return FnKey::F21;
    case GLFW_KEY_F22: return FnKey::F22;
    case GLFW_KEY_F23: return FnKey::F23;
    case GLFW_KEY_F24: return FnKey::F24;
    case GLFW_KEY_F25: return FnKey::F25;
    case GLFW_KEY_PRINT_SCREEN: return FnKey::PrintScreen;
    case GLFW_KEY_SCROLL_LOCK: return FnKey::ScrollLock;
    case GLFW_KEY_PAUSE: return FnKey::Pause;
    case GLFW_KEY_NUM_LOCK: return FnKey::NumLock;
    case GLFW_KEY_KP_0: return FnKey::Numpad0;
    case GLFW_KEY_KP_1: return FnKey::Numpad1;
    case GLFW_KEY_KP_2: return FnKey::Numpad2;
    case GLFW_KEY_KP_3: return FnKey::Numpad3;
    case GLFW_KEY_KP_4: return FnKey::Numpad4;
    case GLFW_KEY_KP_5: return FnKey::Numpad5;
    case GLFW_KEY_KP_6: return FnKey::Numpad6;
    case GLFW_KEY_KP_7: return FnKey::Numpad7;
    case GLFW_KEY_KP_8: return FnKey::Numpad8;
    case GLFW_KEY_KP_9: return FnKey::Numpad9;
    case GLFW_KEY_KP_DECIMAL: return FnKey::NumpadDecimal;
    case GLFW_KEY_KP_DIVIDE: return FnKey::NumpadDivide;
    case GLFW_KEY_KP_MULTIPLY: return FnKey::NumpadMultiply;
    case GLFW_KEY_KP_SUBTRACT: return FnKey::NumpadSubtract;
    case GLFW_KEY_KP_ADD: return FnKey::NumpadAdd;
    case GLFW_KEY_KP_ENTER: return FnKey::NumpadEnter;
    case GLFW_KEY_KP_EQUAL: return FnKey::NumpadEqual;
    case GLFW_KEY_RIGHT_SHIFT: return FnKey::RightShift;
    case GLFW_KEY_LEFT_SHIFT: return FnKey::LeftShift;
    case GLFW_KEY_RIGHT_CONTROL: return FnKey::RightControl;
    case GLFW_KEY_LEFT_CONTROL: return FnKey::LeftControl;
    case GLFW_KEY_RIGHT_ALT: return FnKey::RightAlt;
    case GLFW_KEY_LEFT_ALT: return FnKey::LeftAlt;
    case GLFW_KEY_RIGHT_SUPER: return FnKey::RightSuper;
    case GLFW_KEY_LEFT_SUPER: return FnKey::LeftSuper;
    case GLFW_KEY_MENU: return FnKey::Menu;
    default:
    {
        CRVM_ASSERT_MSG(false, "Unhandled key type!");
        return FnKey::COUNT;
    }
    }
}
using GLFWStateType = decltype(GLFW_PRESS);
KeyStateFlags FromGLFWState(GLFWStateType State)
{
    KeyStateFlags Flags{KeyStateFlagBits::None};
    if (State & GLFW_PRESS) Flags |= KeyStateFlagBits::Pressed;
    if (State & GLFW_RELEASE) Flags |= KeyStateFlagBits::Released;
    if (State & GLFW_REPEAT) Flags |= KeyStateFlagBits::Repeating;
    return Flags;
}

using GLFWModType = decltype(GLFW_MOD_SHIFT);
KeyModFlags FromGLFWMods(GLFWModType Mod)
{
    KeyModFlags Flags{KeyModFlagBits::None};
    if (Mod & GLFW_MOD_SHIFT) Flags |= KeyModFlagBits::Shift;
    if (Mod & GLFW_MOD_CONTROL) Flags |= KeyModFlagBits::Ctrl;
    if (Mod & GLFW_MOD_ALT) Flags |= KeyModFlagBits::Alt;
    if (Mod & GLFW_MOD_SUPER) Flags |= KeyModFlagBits::Super;
    if (Mod & GLFW_MOD_CAPS_LOCK) Flags |= KeyModFlagBits::CapsLock;
    if (Mod & GLFW_MOD_NUM_LOCK) Flags |= KeyModFlagBits::NumLock;
    return Flags;
}

const char* ToString(Key InKey)
{
    switch (InKey)
    {
    case Key::A: return "A";
    case Key::B: return "B";
    case Key::C: return "C";
    case Key::D: return "D";
    case Key::E: return "E";
    case Key::F: return "F";
    case Key::G: return "G";
    case Key::H: return "H";
    case Key::I: return "I";
    case Key::J: return "J";
    case Key::K: return "K";
    case Key::L: return "L";
    case Key::M: return "M";
    case Key::N: return "N";
    case Key::O: return "O";
    case Key::P: return "P";
    case Key::Q: return "Q";
    case Key::R: return "R";
    case Key::S: return "S";
    case Key::T: return "T";
    case Key::U: return "U";
    case Key::V: return "V";
    case Key::W: return "W";
    case Key::X: return "X";
    case Key::Y: return "Y";
    case Key::Z: return "Z";
    case Key::Space: return "Space";
    case Key::Comma: return "Comma";
    case Key::Period: return "Period";
    case Key::Slash: return "Slash";
    case Key::Semicolon: return "Semicolon";
    case Key::Apostrophe: return "Apostrophe";
    case Key::LeftBracket: return "LeftBracket";
    case Key::RightBracket: return "RightBracket";
    case Key::Backslash: return "Backslash";
    case Key::Tilde: return "Tilde";
    case Key::Num0: return "Num0";
    case Key::Num1: return "Num1";
    case Key::Num2: return "Num2";
    case Key::Num3: return "Num3";
    case Key::Num4: return "Num4";
    case Key::Num5: return "Num5";
    case Key::Num6: return "Num6";
    case Key::Num7: return "Num7";
    case Key::Num8: return "Num8";
    case Key::Num9: return "Num9";
    case Key::Minus: return "Minus";
    case Key::Equal: return "Equal";
    case Key::World1: return "World1";
    case Key::World2: return "World2";
    case Key::COUNT:
        [[fallthrough]];
    default:
        return "";
    }
}

}
