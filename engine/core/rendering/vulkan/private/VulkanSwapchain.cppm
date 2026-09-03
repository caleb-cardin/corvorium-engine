// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "EASTL/unique_ptr.h"
#include "EASTL/vector.h"

#include "vulkan/vulkan.hpp"
module Rendering:VulkanSwapchainImpl;
import :VulkanSwapchain;
import :VulkanCore;

namespace Corvorium::Rendering::Vulkan {
VulkanSwapchain::VulkanSwapchain(const VulkanSwapchainCreateParams& Params)
{
    CRVM_ASSERT(Core);
    CRVM_ASSERT(Params.Window);
    Surface = Util::CreateWindowSurface(Params.Instance, Window);

    vk::PhysicalDevice physicalDevice = Core->GetPhysicalDevice();
    vk::ResultValue result = physicalDevice.getSurfaceSupportKHR(Params.PresentQueue, Surface);
    if (!result.has_value())
    {
        CRVM_LOG(VulkanLogger, kError, "Swapchain present not supported for provided queue family.");
        Params.Instance.destroySurfaceKHR(Surface);
        return;
    }

    Initialized = Initialize(Params);
}

VulkanSwapchain::VulkanSwapchain(vk::SurfaceKHR InSurface)
    : Surface{InSurface}
{}

VulkanSwapchain::~VulkanSwapchain()
{
    Teardown();
}

bool VulkanSwapchain::Initialize(const VulkanSwapchainCreateParams& Params)
{
    Window = Params.Window;
    Core = Params.Core;
    QueueFamilyIndices = {Params.GraphicsQueue, Params.PresentQueue};

    OS::WindowCallbackData CallbackData{
        .ResizeFn = OnWindowResized,
        .ResizeUserPtr = this,
        .CloseFn = VulkanCore::OnWindowClosed,
        .CloseUserPtr = Core
    };
    Params.Window->SetCallbackData(CallbackData);

    vk::CommandPoolCreateInfo cmdPoolInfo;
    cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    cmdPoolInfo.queueFamilyIndex = QueueFamilyIndices.Queues.Graphics;

    vk::Device vkDevice = Core->GetDevice();
    for (auto& cmdPool: FrameCommandPools)
    {
        vk::ResultValue result = vkDevice.createCommandPool(cmdPoolInfo);
        CRVM_LOG_RETURN_IF(!result.has_value(), false, VulkanLogger, kError, "Failed to create swapchain command pools.");
        cmdPool = result.value;
    }

    if (CreateOrRecreateSwapchain() != RenderError::Success)
    {
        CRVM_LOG(VulkanLogger, kError, "Failed to create swapchain.");
        Teardown();
        return false;
    }

    if (!CreateSyncPrimitives())
    {
        CRVM_LOG(VulkanLogger, kError, "Failed to create swapchain sync primitives.");
        Teardown();
        return false;
    }

    Initialized = true;
    return true;
}

void VulkanSwapchain::Teardown()
{
    if (Initialized && Core)
    {
        vk::Device vkDevice = Core->GetDevice();

        Reset();

        for (auto view: ImageViews)
        {
            vkDevice.destroyImageView(view);
        }
        ImageViews.clear();
        Images.clear();

        for (auto& pDepth: DepthImages)
        {
            pDepth.reset();
        }

        if (Swapchain) vkDevice.destroySwapchainKHR(Swapchain);

        for (const auto& cmdPool: FrameCommandPools)
        {
            if (cmdPool) vkDevice.destroyCommandPool(cmdPool);
        }

        for (const auto& fence: InFlightFences)
        {
            if (fence) vkDevice.destroyFence(fence);
        }
        for (const auto& semaphore: ImageAvailableSemaphores)
        {
            if (semaphore) vkDevice.destroySemaphore(semaphore);
        }
        for (const auto& semaphore: FrameCompleteSemaphores)
        {
            vkDevice.destroySemaphore(semaphore);
        }

        if (vk::Instance vulkanInstance = Core->GetInstance(); Surface)
        {
            vulkanInstance.destroySurfaceKHR(Surface);
        }
        Initialized = false;
    }
}

void VulkanSwapchain::GetFramebufferSize(u32& Width, u32& Height) const
{
    return Window->GetFramebufferSize(Width, Height);
}

RenderError VulkanSwapchain::CreateOrRecreateSwapchain()
{
    // TEMPORARY FIX TODO: Correctly handle swapchain resizing to avoid images destroy-during-use.
    vk::Device vkDevice = Core->GetDevice();
    vkDevice.waitIdle();


    Initialized = false;

    if (Swapchain)
    {
        Images.clear();
        for (auto view: ImageViews)
        {
            vkDevice.destroyImageView(view);
        }
        ImageViews.clear();

        for (auto& pDepthImage: DepthImages)
        {
            pDepthImage.reset();
        }
    }

    // Surface query

    const Util::SurfaceInfo& surfaceInfo = Core->GetSurfaceInfo();

    vk::PhysicalDeviceSurfaceInfo2KHR surfaceInfo2;
    surfaceInfo2.surface = Surface;
    vk::SurfaceCapabilities2KHR surfaceCaps;
    vk::Result res = Core->GetPhysicalDevice().getSurfaceCapabilities2KHR(&surfaceInfo2, &surfaceCaps, VulkanDispatcher);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, RenderError::InternalError, VulkanLogger, kError, "Failed to get surface capabilities.");

    vk::Extent2D extent = surfaceCaps.surfaceCapabilities.currentExtent;
    if (surfaceCaps.surfaceCapabilities.currentExtent.width == UINT32_MAX ||
        surfaceCaps.surfaceCapabilities.currentExtent.height == UINT32_MAX)
    {
        GetFramebufferSize(extent.width, extent.height);

        extent.width = eastl::clamp(extent.width,
                                    surfaceCaps.surfaceCapabilities.minImageExtent.width,
                                    surfaceCaps.surfaceCapabilities.maxImageExtent.width);
        extent.height = eastl::clamp(extent.height,
                                     surfaceCaps.surfaceCapabilities.minImageExtent.height,
                                     surfaceCaps.surfaceCapabilities.maxImageExtent.height);
    }
    if (extent.width == 0 || extent.height == 0)
    {
        return RenderError::WindowMinimized;
    }

    // Swapchain creation

    vk::SwapchainCreateInfoKHR info;
    info.surface = Surface;
    info.minImageCount = std::min(surfaceCaps.surfaceCapabilities.minImageCount + 1, surfaceCaps.surfaceCapabilities.maxImageCount);
    info.imageFormat = surfaceInfo.ColorFormat.format;
    info.imageColorSpace = surfaceInfo.ColorFormat.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    info.imageSharingMode = IsPresentQueueUnique() ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive;
    info.queueFamilyIndexCount = IsPresentQueueUnique() ? 2 : 1;
    info.pQueueFamilyIndices = QueueFamilyIndices.QueueFamilies;
    info.preTransform = surfaceCaps.surfaceCapabilities.currentTransform;
    info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    info.presentMode = surfaceInfo.PresentMode;
    info.clipped = true;
    info.oldSwapchain = Swapchain;

    const vk::ResultValue scResult = vkDevice.createSwapchainKHR(info);
    CRVM_LOG_RETURN_IF(!scResult.has_value(), RenderError::InitializationFailed, VulkanLogger, kError, "Failed to create swapchain.");
    Swapchain = scResult.value;
    CurrentExtent = extent;
    // Swapchain Color attachments

    u32 imageCount;
    res = vkDevice.getSwapchainImagesKHR(Swapchain, &imageCount, nullptr);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, RenderError::InternalError, VulkanLogger, kError, "Failed to get swapchain images.");
    Images.resize(imageCount);
    res = vkDevice.getSwapchainImagesKHR(Swapchain, &imageCount, Images.data());
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, RenderError::InternalError, VulkanLogger, kError, "Failed to get swapchain images.");

    ImageViews.resize(imageCount);
    for (u32 i = 0; const auto& image: Images)
    {
        vk::ImageViewCreateInfo viewInfo;
        viewInfo.image = image;
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = surfaceInfo.ColorFormat.format;
        viewInfo.components = vk::ComponentMapping{};
        viewInfo.subresourceRange = vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

        if (auto viewResult = vkDevice.createImageView(viewInfo); viewResult.has_value())
        {
            CRVM_LOG_RETURN_IF(!viewResult.has_value(),
                               RenderError::InitializationFailed,
                               VulkanLogger,
                               kError,
                               "Failed to create swapchain image views.");
            ImageViews[i] = viewResult.value;
        }
        ++i;
    }

    // Swapchain depth stencil attachments
    ImageCreateParams params;
    params.Core = Core;
    params.Extent = vk::Extent3D{extent.width, extent.height, 1};
    params.Format = surfaceInfo.DepthStencilFormat;
    params.QueueFamilies = IsPresentQueueUnique()
                               ? QueueFamilyIndices.QueueFamilies
                               : eastl::span<u32>(std::begin(QueueFamilyIndices.QueueFamilies), 1);
    params.Usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

    for (auto& pDepthImage: DepthImages)
    {
        ResultValue<VulkanImage> depthResult = VulkanImage::Create(params);
        CRVM_LOG_RETURN_IF(!depthResult.has_value(),
                           RenderError::InitializationFailed,
                           VulkanLogger,
                           kError,
                           "Failed to create depth attachment images.");
        pDepthImage = eastl::make_unique<VulkanImage>(eastl::move(depthResult.value()));
    }

    bIsOutOfDate = false;

    return RenderError::Success;
}

bool VulkanSwapchain::CreateSyncPrimitives()
{
    vk::Device vkDevice = Core->GetDevice();
    vk::FenceCreateInfo fenceInfo{};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    for (auto& fence: InFlightFences)
    {
        auto result = vkDevice.createFence(fenceInfo);
        if (!result.has_value()) return false;
        fence = result.value;
    }
    vk::SemaphoreCreateInfo semaphoreInfo{};
    for (auto& semaphore: ImageAvailableSemaphores)
    {
        auto result = vkDevice.createSemaphore(semaphoreInfo);
        if (!result.has_value()) return false;
        semaphore = result.value;
    }
    FrameCompleteSemaphores.resize(Images.size());
    for (auto& semaphore: FrameCompleteSemaphores)
    {
        auto result = vkDevice.createSemaphore(semaphoreInfo);
        if (!result.has_value()) return false;
        semaphore = result.value;
    }
    return true;
}

void VulkanSwapchain::OnWindowResized(u32 Width, u32 Height, void* Swapchain)
{
    auto* swapchain = static_cast<VulkanSwapchain*>(Swapchain);
    swapchain->bIsOutOfDate = true;
    if (Width == 0 || Height == 0)
        swapchain->bIsMinimized = true;
    else
        swapchain->bIsMinimized = false;
}

vk::Result VulkanSwapchain::WaitForFence() const
{
    vk::Device vkDevice = Core->GetDevice();
    vk::Result res = vkDevice.waitForFences(1, &InFlightFences[CurrentFrame], true, kMaxTimeout);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to wait for swapchain fences.");
    res = vkDevice.resetFences(1, &InFlightFences[CurrentFrame]);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to reset swapchain fences.");
    return vk::Result::eSuccess;
}

vk::Result VulkanSwapchain::AcquireImage()
{
    vk::Device vkDevice = Core->GetDevice();
    vk::ResultValue result = vkDevice.acquireNextImageKHR(Swapchain, kMaxTimeout, ImageAvailableSemaphores[CurrentFrame]);
    if (result.result == vk::Result::eSuboptimalKHR) bIsOutOfDate = true;
    CRVM_LOG_RETURN_IF(!result.has_value(),
                       result.result,
                       VulkanLogger,
                       kError,
                       "Failed to acquire swapchain image.");
    LastAcquiredImage = result.value;
    return vk::Result::eSuccess;
}

vk::Result VulkanSwapchain::Reset()
{
    vk::Device vkDevice = Core->GetDevice();
    vk::Result res = vkDevice.resetCommandPool(FrameCommandPools[CurrentFrame], vk::CommandPoolResetFlagBits::eReleaseResources);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to reset swapchain frame data.");
    FrameCommandLists[CurrentFrame].CommandBuffers.clear();
    return vk::Result::eSuccess;
}

vk::Result VulkanSwapchain::InitializeCommandList()
{
    vk::CommandBuffer cbBegin;
    vk::Result res = AllocateCommandBuffer(kOrderPriorityMax, cbBegin);

    vk::CommandBufferBeginInfo beginInfo;
    vk::Result beginResult = cbBegin.begin(beginInfo);
    CRVM_LOG_RETURN_IF(beginResult != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to being command recording.");

    // Pipeline barrier
    eastl::array<vk::ImageMemoryBarrier2, 2> barriers{};
    vk::ImageMemoryBarrier2& imgBarrier = barriers[0];
    imgBarrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    imgBarrier.srcAccessMask = vk::AccessFlagBits2::eNone;
    imgBarrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    imgBarrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    imgBarrier.oldLayout = vk::ImageLayout::eUndefined;
    imgBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imgBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    imgBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    imgBarrier.image = GetImage(LastAcquiredImage);
    imgBarrier.subresourceRange = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eColor,
        0u,
        1u,
        0u,
        1u
    };

    vk::ImageMemoryBarrier2& depthBarrier = barriers[1];
    depthBarrier.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe;
    depthBarrier.srcAccessMask = vk::AccessFlagBits2::eNone;
    depthBarrier.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests;
    depthBarrier.dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
    depthBarrier.oldLayout = vk::ImageLayout::eUndefined;
    depthBarrier.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    depthBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    depthBarrier.image = DepthImages[CurrentFrame]->GetImage();
    depthBarrier.subresourceRange = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
        0u,
        1u,
        0u,
        1u
    };

    vk::DependencyInfo beginDep;
    beginDep.imageMemoryBarrierCount = static_cast<u32>(barriers.size());
    beginDep.pImageMemoryBarriers = barriers.data();

    cbBegin.pipelineBarrier2(beginDep);

    cbBegin.end();

    // END
    vk::CommandBuffer cbEnd;
    res = AllocateCommandBuffer(kOrderPriorityMin, cbEnd);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to being command recording.");
    beginResult = cbEnd.begin(beginInfo);
    CRVM_LOG_RETURN_IF(beginResult != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to being command recording.");

    vk::ImageMemoryBarrier2 imgBarrierEnd;
    imgBarrierEnd.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    imgBarrierEnd.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
    imgBarrierEnd.dstStageMask = vk::PipelineStageFlagBits2::eNone;
    imgBarrierEnd.dstAccessMask = vk::AccessFlagBits2::eNone;
    imgBarrierEnd.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
    imgBarrierEnd.newLayout = vk::ImageLayout::ePresentSrcKHR;
    imgBarrierEnd.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    imgBarrierEnd.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    imgBarrierEnd.subresourceRange = vk::ImageSubresourceRange{
        vk::ImageAspectFlagBits::eColor,
        0u, 1u, 0u, 1u
    };

    imgBarrierEnd.image = GetImage(LastAcquiredImage);

    vk::DependencyInfo endDep;
    endDep.imageMemoryBarrierCount = 1;
    endDep.pImageMemoryBarriers = &imgBarrierEnd;

    cbEnd.pipelineBarrier2(endDep);

    cbEnd.end();

    return vk::Result::eSuccess;
}

vk::Result VulkanSwapchain::AllocateCommandBuffer(i32 Priority, vk::CommandBuffer& OutCmdBuf)
{
    vk::CommandBufferAllocateInfo info;
    info.commandPool = FrameCommandPools[CurrentFrame];
    info.commandBufferCount = 1;

    vk::Device device = Core->GetDevice();
    vk::CommandBuffer Cb;
    vk::Result res = device.allocateCommandBuffers(&info, &Cb);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to allocate command buffer.");
    FrameCommandLists[CurrentFrame].CommandBuffers.emplace_back(FrameCommandList::OrderedBuffer{Priority, Cb});
    OutCmdBuf = Cb;
    return vk::Result::eSuccess;
}

vk::Result VulkanSwapchain::CompleteFrame(vk::Queue SubmitQueue)
{
    FrameCommandLists[CurrentFrame].Sort();
    const eastl::vector<FrameCommandList::OrderedBuffer>& cmdListBuffers = FrameCommandLists[CurrentFrame].CommandBuffers;

    eastl::vector<vk::CommandBuffer> cmdBuffers;
    cmdBuffers.resize(cmdListBuffers.size());
    eastl::transform(cmdListBuffers.begin(),
                     cmdListBuffers.end(),
                     cmdBuffers.begin(),
                     [](const FrameCommandList::OrderedBuffer& In) {
                         return In.Cb;
                     });

    vk::PipelineStageFlags waitFlag = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &ImageAvailableSemaphores[CurrentFrame];
    submitInfo.pWaitDstStageMask = &waitFlag;
    submitInfo.commandBufferCount = static_cast<u32>(cmdBuffers.size());
    submitInfo.pCommandBuffers = cmdBuffers.data();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &FrameCompleteSemaphores[LastAcquiredImage];

    vk::Result res = SubmitQueue.submit(submitInfo, InFlightFences[CurrentFrame]);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, res, VulkanLogger, kError, "Failed to submit commands to queue.");

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &FrameCompleteSemaphores[LastAcquiredImage];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = Swapchain.Ref();
    presentInfo.pImageIndices = &LastAcquiredImage;
    presentInfo.pResults = &res;

    res = SubmitQueue.presentKHR(presentInfo);
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR,
                       res,
                       VulkanLogger,
                       kError,
                       "Failed to submit commands to queue.");


    CurrentFrame = (CurrentFrame + 1) % kMaxFramesInFlight;
    return vk::Result::eSuccess;
}

void VulkanSwapchain::FillRenderingInfo(vk::RenderingAttachmentInfo& ColorAttachment,
                                        vk::RenderingAttachmentInfo& DepthAttachment,
                                        vk::RenderingInfo& RenderInfo) const
{
    ColorAttachment.imageView = GetImageView(LastAcquiredImage);
    ColorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    // TODO : MSAA
    ColorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    ColorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    ColorAttachment.clearValue = vk::ClearColorValue{0.f, 0.f, 0.f, 1.f};

    DepthAttachment.imageView = DepthImages[CurrentFrame]->GetImageView();
    DepthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    DepthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    DepthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    DepthAttachment.clearValue = vk::ClearDepthStencilValue{1.f, 0};


    RenderInfo.renderArea = vk::Rect2D{{0, 0}, CurrentExtent};
    RenderInfo.layerCount = 1;
    RenderInfo.colorAttachmentCount = 1;
    RenderInfo.pColorAttachments = &ColorAttachment;
    RenderInfo.pDepthAttachment = &DepthAttachment;
}
}
