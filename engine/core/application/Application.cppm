// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "EASTL/string.h"
#include "EASTL/unique_ptr.h"
export module Application;
import Rendering;
import OS;

export namespace Corvorium {
class CRVM_API Application {
public:
    explicit Application(eastl::string&& AppName);
    virtual ~Application() = default;

    [[nodiscard]] bool IsInitialized() const;

    void Run();

protected:
    virtual void SomethingElse() = 0;

private:
    eastl::string Name;
    eastl::unique_ptr<OS::WindowManager> WindowManager{nullptr};
    eastl::unique_ptr<Rendering::RenderContext> Rendering{nullptr};

    bool Initialized{false};

    OS::WindowHandle MainWindowHandle{};
};

Application::Application(eastl::string&& AppName)
    : Name{eastl::move(AppName)}
    , WindowManager{eastl::make_unique<Corvorium::OS::WindowManager>()}
    , Rendering{Rendering::RenderContext::CreateRenderContext(Rendering::RenderingAPI::Vulkan)}
{
    MainWindowHandle = WindowManager->NewWindow(1600, 900, Name.c_str());
    if (MainWindowHandle == OS::WindowHandle::kInvalidHandle)
    {
        Rendering.reset();
        return;
    }

    if (Rendering->Initialize(WindowManager->GetWindow(MainWindowHandle)) != Rendering::RenderError::Success)
    {
        return;
    }

    Initialized = true;
}

bool Application::IsInitialized() const
{
    return Initialized;
}

void Application::Run()
{
    while (!WindowManager->IsWindowClosed(MainWindowHandle))
    {
        OS::Window::PollEvents();
    }
}
}
