// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "CorvoriumApi.h"

#include "EASTL/string.h"
#include "EASTL/unique_ptr.h"

#include "common/Logging.hpp"
export module Application;
import Logging;
import Event;
import OS;
import Rendering;
import VFS;
import std;

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
    eastl::unique_ptr<Event::GlobalEventBus> EventBus{nullptr};
    eastl::unique_ptr<OS::WindowManager> WindowManager{nullptr};
    eastl::unique_ptr<Rendering::IRenderCore> Rendering{nullptr};
    eastl::unique_ptr<VFS::VirtualFileSystem> FileSystem{nullptr};

    bool Initialized{false};

    OS::WindowHandle MainWindowHandle{};
};

Application::Application(eastl::string&& AppName)
    : Name{eastl::move(AppName)}
    , AppLogger{Name}
    , EventBus{eastl::make_unique<Event::GlobalEventBus>()}
    , WindowManager{eastl::make_unique<OS::WindowManager>(EventBus.get())}
    , Rendering{Rendering::IRenderCore::CreateRenderCore(Rendering::RenderingAPI::Vulkan)}
    , FileSystem{eastl::make_unique<VFS::VirtualFileSystem>()}
{
    MainWindowHandle = WindowManager->NewWindow(1600, 900, Name.c_str());
    if (MainWindowHandle == OS::WindowHandle::kInvalidHandle)
    {
        Rendering.reset();
        return;
    }

    if (Rendering->Initialize(EventBus.get(), WindowManager->GetWindow(MainWindowHandle), FileSystem.get()) != Rendering::RenderError::Success)
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
    eastl::string physicalPath = (std::filesystem::current_path()
                                  / std::filesystem::path("resources")
                                  / std::filesystem::path("shaders")).string().c_str();
    FileSystem->Mount(physicalPath, "shaders");

    Rendering::ResultValue<Rendering::ShaderModuleHandle> shaderRes = Rendering->CreateShaderModule("shaders://Test.slang");
    if (!shaderRes.has_value()) return;

    Rendering::GraphicsPipelineCreateParams graphicsParams{};
    graphicsParams.Module = shaderRes.value();
    graphicsParams.ColorBlend = Rendering::ColorBlendType::None;
    graphicsParams.DepthTest = Rendering::DepthTestType::Opaque;
    graphicsParams.Primitive = Rendering::PrimitiveType::TriangleList;

    Rendering::ResultValue<Rendering::PipelineHandle> pipelineRes = Rendering->CreatePipeline(graphicsParams);
    if (!pipelineRes.has_value()) return;

    while (!WindowManager->IsWindowClosed(MainWindowHandle))
    {
        OS::WindowManager::PollEvents();

        if (!WindowManager->IsWindowClosed(MainWindowHandle) &&
            Rendering->BeginFrame(MainWindowHandle) == Rendering::RenderError::Success)
        {
            auto cb = Rendering->AllocateCommandBuffer(MainWindowHandle, 0);
            if (cb.has_value() && cb->Begin())
            {
                cb->BeginRendering();
                cb->BindGraphicsPipeline(pipelineRes.value());
                cb->TESTING_DrawTriangle();
                cb->EndRendering();
                cb->End();
            }

            Rendering->EndFrame(MainWindowHandle);
        }
    }
}
}
