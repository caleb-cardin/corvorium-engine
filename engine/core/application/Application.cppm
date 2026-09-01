// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "CorvoriumApi.h"

#include "EASTL/string.h"
#include "EASTL/unique_ptr.h"

#include <filesystem>
#include <print>

#include "common/Logging.hpp"
export module Application;
import Common;
import OS;
import Rendering;
import VFS;

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
    Log::Logger AppLogger;
    eastl::unique_ptr<OS::WindowManager> WindowManager{nullptr};
    eastl::unique_ptr<Rendering::IRenderCore> Rendering{nullptr};
    eastl::unique_ptr<VFS::VirtualFileSystem> FileSystem{nullptr};

    bool Initialized{false};

    OS::WindowHandle MainWindowHandle{};
};

Application::Application(eastl::string&& AppName)
    : Name{eastl::move(AppName)}
    , AppLogger{Name}
    , WindowManager{eastl::make_unique<Corvorium::OS::WindowManager>()}
    , Rendering{Rendering::IRenderCore::CreateRenderCore(Rendering::RenderingAPI::Vulkan)}
    , FileSystem{eastl::make_unique<VFS::VirtualFileSystem>()}
{
    MainWindowHandle = WindowManager->NewWindow(1600, 900, Name.c_str());
    if (MainWindowHandle == OS::WindowHandle::kInvalidHandle)
    {
        Rendering.reset();
        return;
    }

    if (Rendering->Initialize(WindowManager->GetWindow(MainWindowHandle), FileSystem.get()) != Rendering::RenderError::Success)
    {
        return;
    }

    eastl::string physicalPath = (std::filesystem::current_path()
                                  / std::filesystem::path("resources")
                                  / std::filesystem::path("shaders")).string().c_str();
    FileSystem->Mount(physicalPath, "shaders");

    Rendering::ResultValue<Rendering::ShaderModuleHandle> shaderRes = Rendering->CreateShaderModule("shaders://Test.slang");
    if (!shaderRes.has_value())
    {
        CRVM_LOG(AppLogger, kInfo, "Shader creation failed");
    }
    else
    {
        CRVM_LOG(AppLogger, kInfo, "Shader creation succeeded");
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
