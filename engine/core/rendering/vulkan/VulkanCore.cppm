// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/hash_map.h"
#include "EASTL/vector_map.h"
#include "EASTL/string.h"
#include "vulkan/vulkan.hpp"
#include "vma/vk_mem_alloc.h"
export module Rendering:VulkanCore;
import :RenderCore;
import :VulkanSwapchain;
import :VulkanPipeline;
import :VulkanShaderCache;

namespace Corvorium::Rendering::Vulkan {
class VulkanCore : public IRenderCore {
    struct QueueData {
        u32 FamilyIndex{};
        u32 QueueIndex{};
        vk::Queue Queue{};
        vk::QueueFlags Flags{};
    };

public:
    VulkanCore() = default;
    ~VulkanCore() override;

    [[nodiscard]] RenderError Initialize(Event::GlobalEventBus* EventHandler,
                                         OS::Window* MainWindow,
                                         FileSystem::IFileProvider* InFileProvider) override;

    [[nodiscard]] bool IsInitialized() const override { return Initialized; }

    [[nodiscard]] RenderError InitializeWindow(OS::Window* Window) override;

    [[nodiscard]] RenderError BeginFrame(OS::WindowHandle Window) override;
    [[nodiscard]] ResultValue<CommandBuffer> AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority) override;
    [[nodiscard]] RenderError EndFrame(OS::WindowHandle Window) override;

    [[nodiscard]] ResultValue<ShaderModuleHandle> CreateShaderModule(const eastl::string& Name) override;
    [[nodiscard]] ResultValue<PipelineHandle> CreatePipeline(const GraphicsPipelineCreateParams& Params) override;

    [[nodiscard]] vk::Instance GetInstance() const { return Instance; }
    [[nodiscard]] vk::PhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
    [[nodiscard]] vk::Device GetDevice() const { return Device; }
    [[nodiscard]] VmaAllocator GetAllocator() const { return Allocator; }

    [[nodiscard]] const Util::SurfaceInfo& GetSurfaceInfo() const { return SurfaceInfo; };

    [[nodiscard]] const VulkanPipeline* FindPipeline(PipelineHandle Pipeline);

private:
    Event::GlobalEventBus* EventBus{nullptr};
    FileSystem::IFileProvider* FileProvider{nullptr};
    vk::Instance Instance{nullptr};
    vk::DebugUtilsMessengerEXT DebugMessenger{nullptr};
    bool Initialized{false};

    vk::PhysicalDevice PhysicalDevice{nullptr};
    vk::Device Device{nullptr};

    VmaAllocator Allocator{nullptr};

    vk::PipelineCache PipelineCache{nullptr};

    QueueData GraphicsQueue{};
    QueueData TransferQueue{};
    u32 PresentQueueFamily{};

    Util::SurfaceInfo SurfaceInfo{};
    eastl::vector_map<OS::WindowHandle, VulkanSwapchain> Swapchains{};

    eastl::unique_ptr<VulkanShaderCache> ShaderCache{};

    eastl::hash_map<PipelineHandle, VulkanPipeline, HandleHash> Pipelines{};

    [[nodiscard]] bool InitInstance();
    [[nodiscard]] bool SelectPhysicalDevice();
    [[nodiscard]] bool SelectSurfaceFormats(vk::SurfaceKHR Surface);
    [[nodiscard]] bool InitDevice();
    [[nodiscard]] bool InitAllocator(const eastl::vector<const char*>& EnabledExtensionNames);
    [[nodiscard]] bool InitPipelineCache();

    void Teardown();

    bool HandleFramebufferResized(const OS::WindowFramebufferResizeEvent& Event);
    bool HandleWindowClosed(const OS::WindowCloseEvent& Event);

    bool FindPresentQueueFamily(vk::SurfaceKHR Surface, u32& OutPresentFamily) const;
    eastl::vector<QueueData> CreateQueueInfos(eastl::vector<vk::DeviceQueueCreateInfo>& OutQueueInfos) const;
    [[nodiscard]] bool IsTransferQueueConcurrent() const;

    [[nodiscard]] eastl::vector<u8> LoadPipelineCacheData() const;
};
}
