// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/FlagTypeMacro.hpp"

#include "EASTL/array.h"

#include "GLFW/glfw3.h"
export module OS:Input;
import :InputTypes;
import :InputConversionHelpers;

namespace Corvorium::Input {

struct KeyState {
    KeyStateFlags State{};
    KeyModFlags Mods{};
};

struct AxisState {
    KeyStateFlags Flags{};
    f32 Value{};
};

struct InputState {
    eastl::array<KeyState, static_cast<u8>(Key::COUNT)> KeyStates{};
    eastl::array<KeyState, static_cast<u8>(FnKey::COUNT)> FnKeyStates{};
    eastl::array<KeyState, static_cast<u8>(MouseButton::COUNT)> MouseButtonStates{};
    eastl::array<KeyState, static_cast<u8>(GamepadButton::COUNT)> GamepadButtonStates{};
    eastl::array<AxisState, static_cast<u8>(GamepadAxis::COUNT)> GamepadAxisStates{};
};

}
