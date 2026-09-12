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


WindowManager::WindowManager(InputManager* InputManager, Event::GlobalEventBus* EventBus)
    : Windows{}
{
    CRVM_ASSERT(EventBus);
    if (glfwInit())
    {
        Initialized = true;
    }

    TemplateCallbackData.pWindowManager = this;
    TemplateCallbackData.pInputManager = InputManager;
    TemplateCallbackData.pEventBus = EventBus;
}

WindowManager::~WindowManager()
{
    Windows.clear();
    glfwTerminate();
}

WindowHandle WindowManager::NewWindow(u32 Width, u32 Height, const char* Title)
{
    auto window = eastl::unique_ptr<Window>(new Window(TemplateCallbackData, Width, Height, Title));
    auto [it, success] = Windows.try_emplace(window->GetHandle(), eastl::move(window));
    if (success)
    {
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
}
