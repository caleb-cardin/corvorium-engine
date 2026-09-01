// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/sort.h"
#include "EASTL/span.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/vector.h"

#include "vulkan/vulkan.hpp"

#include <limits>

module Rendering:VulkanSwapchain;
import OS;
import Common;
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
    inline static constexpr i32 kOrderPriorityMin = std::numeric_limits<i32>::min();
    inline static constexpr i32 kOrderPriorityMax = std::numeric_limits<i32>::max();

    static VulkanSwapchain CreateUninitializedSwapchain(vk::SurfaceKHR FromSurface) { return VulkanSwapchain{FromSurface}; }
    static VulkanSwapchain CreateInitializedSwapchain(const VulkanSwapchainCreateParams& Params) { return VulkanSwapchain{Params}; }

    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&) noexcept = default;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&) noexcept = default;
    VulkanSwapchain() = delete;
    ~VulkanSwapchain();

    [[nodiscard]] bool Initialize(const VulkanSwapchainCreateParams& Params);
    [[nodiscard]] bool IsInitialized() const { return Initialized; }
    void Teardown();

    [[nodiscard]] vk::SwapchainKHR GetSwapchain() const { return Swapchain; }
    [[nodiscard]] vk::SurfaceKHR GetSurface() const { return Surface; }
    [[nodiscard]] vk::Image GetImage(u32 Index) const { return Images[Index]; }
    [[nodiscard]] vk::ImageView GetImageView(u32 Index) const { return ImageViews[Index]; }

    [[nodiscard]] vk::Image GetDepthImage() { return DepthImages[CurrentFrame] ? DepthImages[CurrentFrame]->GetImage() : nullptr; }
    [[nodiscard]] vk::ImageView GetDepthImageView() { return DepthImages[CurrentFrame] ? DepthImages[CurrentFrame]->GetImageView() : nullptr; }

    void GetFramebufferSize(u32& Width, u32& Height) const;

    [[nodiscard]] RenderError CreateOrRecreateSwapchain();
    [[nodiscard]] bool IsOutOfDate() const { return bIsOutOfDate; }
    [[nodiscard]] bool IsMinimized() const { return bIsMinimized; }

    static void OnWindowResized(u32 Width, u32 Height, void* Swapchain);

    [[nodiscard]] vk::Result WaitForFence() const;
    [[nodiscard]] vk::Result AcquireImage();
    [[nodiscard]] vk::Result Reset();

    [[nodiscard]] vk::Result InitializedCommandList();
    [[nodiscard]] vk::Result AllocateCommandBuffer(i32 Priority, vk::CommandBuffer& OutCmdBuf);
    [[nodiscard]] vk::Result CompleteFrame(vk::Queue SubmitQueue);

private:
    VulkanCore* Core{nullptr};
    OS::Window* Window{nullptr};

    vk::SwapchainKHR Swapchain{nullptr};
    vk::SurfaceKHR Surface{nullptr};
    eastl::vector<vk::Image> Images{};
    eastl::vector<vk::ImageView> ImageViews{};

    eastl::array<eastl::unique_ptr<VulkanImage>, kMaxFramesInFlight> DepthImages;

    QueueFamilyIndices QueueFamilyIndices;

    u32 CurrentFrame{0};
    u32 LastAcquiredImage{~0x0u};
    eastl::array<vk::CommandPool, kMaxFramesInFlight> FrameCommandPools{nullptr, nullptr};
    eastl::array<FrameCommandList, kMaxFramesInFlight> FrameCommandLists{};
    eastl::array<vk::Fence, kMaxFramesInFlight> InFlightFences{nullptr, nullptr};
    eastl::array<vk::Semaphore, kMaxFramesInFlight> ImageAvailableSemaphores{nullptr, nullptr};
    eastl::vector<vk::Semaphore> FrameCompleteSemaphores{};

    bool Initialized{false};
    bool bIsOutOfDate{false};
    bool bIsMinimized{false};

    explicit VulkanSwapchain(const VulkanSwapchainCreateParams& Params);
    explicit VulkanSwapchain(vk::SurfaceKHR InSurface);

    bool CreateSyncPrimitives();

    [[nodiscard]] bool IsPresentQueueUnique() const { return QueueFamilyIndices.Queues.Graphics != QueueFamilyIndices.Queues.Present; }
};
}
