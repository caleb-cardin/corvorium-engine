// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Logging.hpp"

#include "EASTL/vector.h"
#include "EASTL/array.h"
#include "EASTL/hash_map.h"
#include "EASTL/variant.h"

#include "vulkan/vulkan.hpp"
module Rendering:VkDevice;
import :Device;
import :Types;
import :VkSwapchain;
import :VkPipeline;
import :VkUtils;

namespace Corvorium::Rendering::Vulkan {
class VulkanCommandBuffer;


class VulkanDevice : public RenderDevice {
    struct QueueData {
        u32 FamilyIndex{};
        u32 QueueIndex{};
        vk::Queue Queue{};
        vk::QueueFlags Flags{};
    };

public:
    VulkanDevice() = delete;
    explicit VulkanDevice(vk::Instance Instance, OS::Window* MainWindow);
    ~VulkanDevice() override;

    [[nodiscard]] bool IsInitialized() const override;

    [[nodiscard]] RenderError BeginFrame(OS::WindowHandle Window) override;
    [[nodiscard]] ResultValue<CommandBuffer> AllocateCommandBuffer(OS::WindowHandle Window, i32 Priority);
    [[nodiscard]] RenderError EndFrame(OS::WindowHandle Window) override;

    [[nodiscard]] vk::Instance GetInstance() const { return Instance; }
    [[nodiscard]] vk::PhysicalDevice GetPhysicalDevice() const { return PhysicalDevice; }
    [[nodiscard]] vk::Device GetDevice() const { return Device; }

    bool CreateSwapchainForWindow(OS::Window* InWindow);
    static void OnWindowClosed(OS::Window* Window, void* OwningDevice);


    [[nodiscard]] vk::Pipeline FindPipeline(PipelineHandle Pipeline);

private:
    vk::Instance Instance{nullptr};
    bool Initialized{false};

    vk::PhysicalDevice PhysicalDevice{nullptr};
    vk::Device Device{nullptr};

    vk::PipelineCache PipelineCache{nullptr};

    QueueData GraphicsQueue{};
    QueueData TransferQueue{};
    u32 PresentQueueFamily{};

    eastl::hash_map<OS::WindowHandle, VulkanSwapchain, HandleHash> Swapchains{};

    eastl::hash_map<PipelineHandle, VulkanPipeline, HandleHash> Pipelines{};

    [[nodiscard]] bool SelectPhysicalDevice();
    [[nodiscard]] bool InitDevice();
    [[nodiscard]] bool InitPipelineCache();

    void Teardown();

    bool FindPresentQueueFamily(vk::SurfaceKHR Surface, u32& OutPresentFamily) const;
    eastl::vector<QueueData> CreateQueueInfos(eastl::vector<vk::DeviceQueueCreateInfo>& OutQueueInfos) const;
    [[nodiscard]] bool IsTransferQueueConcurrent() const;

    [[nodiscard]] eastl::vector<u8> LoadPipelineCacheData() const;
};
}
