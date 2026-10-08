// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/hash_map.h"
#include "EASTL/vector_map.h"
#include "EASTL/string.h"

#include "vma/vk_mem_alloc.h"
export module Rendering:VulkanCore;
import vulkan;
import :RenderCore;
import :VulkanSwapchain;
import :VulkanPipeline;
import :VulkanShaderCache;

namespace Corvorium::Rendering::Vulkan {
class VulkanCore final : public IRenderCore {
    struct QueueData {
        u32 FamilyIndex{};
        u32 QueueIndex{};
        vk::Queue Queue{};
        vk::QueueFlags Flags{};
    };

public:
    VulkanCore() = default;

    // ==================================================================
    // IRenderCor overridee
    // ==================================================================

    ~VulkanCore() override;

    [[nodiscard]] RenderError Initialize(Event::GlobalEventBus* EventHandler,
                                         OS::Window* MainWindow,
                                         FileSystem::IFileProvider* InFileProvider) override;

    [[nodiscard]] bool IsInitialized() const override { return Initialized; }

    [[nodiscard]] RenderError InitializeWindow(OS::Window* Window) override;

    [[nodiscard]] RenderError BeginFrame(OS::WindowHandle Window) override;
    [[nodiscard]] ResultValue<CommandBuffer> AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority) override;
    [[nodiscard]] RenderError EndFrame(OS::WindowHandle Window) override;

    [[nodiscard]] ResultValue<ShaderHandle> ShaderCreate(const eastl::string& Name) override;

    [[nodiscard]] ResultValue<PipelineHandle> PipelineCreate(const GraphicsPipelineCreateParams& Params) override;

    [[nodiscard]] inline ResultValue<ImageHandle> ImageCreate(Size2D Size, Format Format, ImageUsageFlags Usage) override;
    [[nodiscard]] ResultValue<ImageHandle> ImageCreate(Size3D Size, Format Format, ImageUsageFlags Usage) override;
    [[nodiscard]] RenderError ImageFillFromMemory(ImageHandle Image, void* Data, Format DataFormat, u64 Size, u64 Offset) override;
    [[nodiscard]] RenderError ImageFillFromMemory(ImageHandle Image, eastl::span<Color> Colors, u64 Offset) override;

    // ==================================================================
    // VulkanCore methods
    // ==================================================================

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
    //eastl::hash_map<BufferHandle, VulkanBuffer, HandleHash> Buffers{}; TODO : Vulkan buffer
    eastl::hash_map<ImageHandle, VulkanImage, HandleHash> Images{};

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
