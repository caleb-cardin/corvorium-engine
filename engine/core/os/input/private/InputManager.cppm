// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

module OS:InputManagerImpl;
import :InputManager;
import :Input;

namespace Corvorium::OS {


void InputManager::UpdateKeyState(const Key Key, const KeyStateFlags StateFlags, const KeyModFlags ModFlags)
{
    bool wasUpdated = WasUpdated(State.KeyStates[static_cast<u8>(Key)].State, StateFlags);
    State.KeyStates[static_cast<u8>(Key)].State = wasUpdated ? StateFlags | KeyStateFlagBits::JustUpdated : StateFlags;
    State.KeyStates[static_cast<u8>(Key)].Mods = ModFlags;
}

void InputManager::UpdateFnKeyState(const FnKey Key, const KeyStateFlags StateFlags, const KeyModFlags ModFlags)
{
    bool wasUpdated = WasUpdated(State.FnKeyStates[static_cast<u8>(Key)].State, StateFlags);
    State.FnKeyStates[static_cast<u8>(Key)].State = wasUpdated ? StateFlags | KeyStateFlagBits::JustUpdated : StateFlags;
    State.FnKeyStates[static_cast<u8>(Key)].Mods = ModFlags;
}

void InputManager::UpdateMouseButtonState(const MouseButton Button, const KeyStateFlags StateFlags, const KeyModFlags ModFlags)
{
    bool wasUpdated = WasUpdated(State.MouseButtonStates[static_cast<u8>(Button)].State, StateFlags);
    State.MouseButtonStates[static_cast<u8>(Button)].State = wasUpdated ? StateFlags | KeyStateFlagBits::JustUpdated : StateFlags;
    State.MouseButtonStates[static_cast<u8>(Button)].Mods = ModFlags;
}



}
