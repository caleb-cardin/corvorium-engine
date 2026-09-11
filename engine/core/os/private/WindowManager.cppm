// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "EASTL/hash_map.h"
#include "EASTL/unique_ptr.h"

#include "GLFW/glfw3.h"
module OS:WindowManagerImpl;
import :WindowManager;
import Logging;

namespace Corvorium::OS {
Log::Logger WindowLogger {"WindowManager"};


WindowManager::WindowManager(Event::GlobalEventBus* InEventBus)
    : EventBus{InEventBus}
, Windows{}
{
    CRVM_ASSERT(EventBus);
    if (glfwInit())
    {
        Initialized = true;
    }
}

WindowManager::~WindowManager()
{
    Windows.clear();
    glfwTerminate();
}

WindowHandle WindowManager::NewWindow(u32 Width, u32 Height, const char* Title)
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

Window* WindowManager::GetWindow(const WindowHandle& Handle)
{
    auto it = Windows.find(Handle);
    if (it != Windows.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void WindowManager::UnregisterWindow(const WindowHandle& Handle)
{
    Windows.erase(Handle);
}

void WindowManager::UpdateKeyState(Input::Key Key, Input::KeyStateFlags StateFlags, Input::KeyModFlags ModFlags)
{
    bool wasUpdated = Input::WasUpdated(InputState.KeyStates[static_cast<u8>(Key)].State, StateFlags);
    if (wasUpdated && (StateFlags & Input::KeyStateFlagBits::Pressed))
    {
        CRVM_LOG(WindowLogger, kInfo, "Key pressed: {}", Input::Internal::ToString(Key));
    }
    InputState.KeyStates[static_cast<u8>(Key)].State = wasUpdated ? StateFlags | Input::KeyStateFlagBits::JustUpdated : StateFlags;
    InputState.KeyStates[static_cast<u8>(Key)].Mods = ModFlags;
}

void WindowManager::UpdateFnKeyState(Input::FnKey Key, Input::KeyStateFlags StateFlags, Input::KeyModFlags ModFlags)
{
    bool wasUpdated = Input::WasUpdated(InputState.FnKeyStates[static_cast<u8>(Key)].State, StateFlags);
    InputState.FnKeyStates[static_cast<u8>(Key)].State = wasUpdated ? StateFlags | Input::KeyStateFlagBits::JustUpdated : StateFlags;
    InputState.FnKeyStates[static_cast<u8>(Key)].Mods = ModFlags;
}

void WindowManager::UpdateMouseButtonState(Input::MouseButton Button, Input::KeyStateFlags StateFlags, Input::KeyModFlags ModFlags)
{
    InputState.FnKeyStates[static_cast<u8>(Button)].State = StateFlags;
    InputState.FnKeyStates[static_cast<u8>(Button)].Mods = ModFlags;
}
}
