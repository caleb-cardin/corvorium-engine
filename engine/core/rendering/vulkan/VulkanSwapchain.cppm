// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/sort.h"
#include "EASTL/span.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/vector.h"

#include <limits>

module Rendering:VulkanSwapchain;
import vulkan;
import :VulkanImage;
import :VulkanUtils;
import :Types;

namespace Corvorium::Rendering::Vulkan {
class VulkanCore;

struct VulkanSwapchainCreateParams {
    vk::Instance Instance;
    VulkanCore* Core;
    OS::Window* Window;
    u32 GraphicsQueue;
    u32 PresentQueue;
};

class VulkanSwapchain {
    union QueueFamilyIndices {
        u32 QueueFamilies[2]{};

        struct Queues {
            u32 Graphics{};
            u32 Present{};
        } Queues;
    };

    struct FrameCommandList {
        struct OrderedBuffer {
            i32 Priority{0};
            vk::CommandBuffer Cb{nullptr};
        };

        eastl::vector<OrderedBuffer> CommandBuffers{};

        void Sort()
        {
            eastl::sort(CommandBuffers.begin(),
                        CommandBuffers.end(),
                        [](const OrderedBuffer& A, const OrderedBuffer& B) -> bool {
                            return A.Priority < B.Priority;
                        });
        }
    };

public:
    inline static constexpr i32 kOrderPriorityMax = std::numeric_limits<i32>::min();
    inline static constexpr i32 kOrderPriorityMin = std::numeric_limits<i32>::max();

    static VulkanSwapchain CreateUninitializedSwapchain(vk::SurfaceKHR FromSurface) { return VulkanSwapchain{FromSurface}; }
    static VulkanSwapchain CreateInitializedSwapchain(const VulkanSwapchainCreateParams& Params) { return VulkanSwapchain{Params}; }

    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&& Other) noexcept = default;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&& Other) noexcept = default;
    ~VulkanSwapchain();

    [[nodiscard]] bool Initialize(const VulkanSwapchainCreateParams& Params);
    [[nodiscard]] bool IsInitialized() const { return Initialized; }
    void Teardown();

    [[nodiscard]] vk::SwapchainKHR GetSwapchain() const { return Swapchain; }
    [[nodiscard]] vk::SurfaceKHR GetSurface() const { return Surface; }
    [[nodiscard]] vk::Image GetImage(u32 Index) const { return Images[Index]; }
    [[nodiscard]] vk::ImageView GetImageView(u32 Index) const { return ImageViews[Index]; }

    [[nodiscard]] vk::Image GetDepthImage() const { return DepthImages[CurrentFrame] ? DepthImages[CurrentFrame]->GetImage() : nullptr; }

    [[nodiscard]] vk::ImageView GetDepthImageView() const
    {
        return DepthImages[CurrentFrame] ? DepthImages[CurrentFrame]->GetImageView() : nullptr;
    }

    [[nodiscard]] vk::Extent2D GetExtent() const { return CurrentExtent; }

    [[nodiscard]] RenderError CreateOrRecreateSwapchain();
    [[nodiscard]] bool IsOutOfDate() const { return bIsOutOfDate; }
    [[nodiscard]] bool IsMinimized() const { return bIsMinimized; }

    void OnWindowResized(u32 Width, u32 Height);

    [[nodiscard]] vk::Result WaitForFence() const;
    [[nodiscard]] vk::Result AcquireImage();
    [[nodiscard]] vk::Result Reset();
    [[nodiscard]] vk::Result ReleaseCommandResources();

    [[nodiscard]] vk::Result InitializeCommandList();
    [[nodiscard]] vk::Result AllocateCommandBuffer(i32 Priority, vk::CommandBuffer& OutCmdBuf);
    [[nodiscard]] vk::Result CompleteFrame(vk::Queue SubmitQueue);

    [[nodiscard]] void FillRenderingInfo(vk::RenderingAttachmentInfo& ColorAttachment,
                                         vk::RenderingAttachmentInfo& DepthAttachment,
                                         vk::RenderingInfo& RenderInfo) const;

private:
    Auto<VulkanCore*> Core{nullptr};
    Auto<OS::Window*> Window{nullptr};

    Auto<vk::SwapchainKHR> Swapchain{nullptr};
    Auto<vk::SurfaceKHR> Surface{nullptr};
    eastl::vector<vk::Image> Images{};
    eastl::vector<vk::ImageView> ImageViews{};

    eastl::array<eastl::unique_ptr<VulkanImage>, kMaxFramesInFlight> DepthImages{};

    QueueFamilyIndices QueueFamilyIndices;

    u32 CurrentFrame{0};
    u32 LastAcquiredImage{~0x0u};
    vk::Extent2D CurrentExtent{};

    eastl::array<vk::CommandPool, kMaxFramesInFlight> FrameCommandPools{nullptr, nullptr};
    eastl::array<FrameCommandList, kMaxFramesInFlight> FrameCommandLists{};
    eastl::array<vk::Fence, kMaxFramesInFlight> InFlightFences{nullptr, nullptr};
    eastl::array<vk::Semaphore, kMaxFramesInFlight> ImageAvailableSemaphores{nullptr, nullptr};
    eastl::vector<vk::Semaphore> FrameCompleteSemaphores{};

    bool Initialized{false};
    bool bIsOutOfDate{false};
    bool bIsMinimized{false};

    VulkanSwapchain() = default;
    explicit VulkanSwapchain(const VulkanSwapchainCreateParams& Params);
    explicit VulkanSwapchain(vk::SurfaceKHR InSurface);

    bool CreateSyncPrimitives();

    [[nodiscard]] bool IsPresentQueueUnique() const { return QueueFamilyIndices.Queues.Graphics != QueueFamilyIndices.Queues.Present; }
};
}
