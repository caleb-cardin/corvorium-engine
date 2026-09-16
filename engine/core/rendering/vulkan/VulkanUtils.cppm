// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "EASTL/vector.h"

#include "vulkan/vulkan_core.h"
#include "GLFW/glfw3.h"
module Rendering:VulkanUtils;
import vulkan;
import Logging;
import Debug;
import Flags;
import RenderingCommon;
import OS;

namespace vk::detail {
extern DispatchLoaderDynamic defaultDispatchLoaderDynamic;
}

namespace Corvorium::Rendering::Vulkan {
inline auto& VulkanDispatch = vk::detail::defaultDispatchLoaderDynamic;

Log::Logger VulkanLogger{"VulkanRendering"};

constexpr u32 kMaxFramesInFlight = 2;
constexpr u64 kMaxTimeout = UINT64_MAX;

namespace Util {
vk::Bool32 DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    [[maybe_unused]] vk::DebugUtilsMessageTypeFlagsEXT MessageType,
    const vk::DebugUtilsMessengerCallbackDataEXT* CallbackData,
    [[maybe_unused]] void* UserData)
{
    switch (MessageSeverity)
    {
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
        [[fallthrough]];
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
    {
        CRVM_LOG(VulkanLogger, kInfo, "{}", CallbackData->pMessage);
        break;
    }
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
    {
        CRVM_LOG(VulkanLogger, kWarning, "{}", CallbackData->pMessage);
        break;
    }
    case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
    {
        CRVM_LOG(VulkanLogger, kError, "{}", CallbackData->pMessage);
        break;
    }
    default: { break; }
    }
    DebugBreak();
    return false;
}

struct SurfaceInfo {
    vk::SurfaceFormatKHR ColorFormat;
    vk::Format DepthStencilFormat;
    vk::PresentModeKHR PresentMode;
};

vk::SurfaceKHR CreateWindowSurface(const vk::Instance Instance, const OS::Window* Window)
{
    vk::SurfaceKHR surface;
    auto res = static_cast<vk::Result>(glfwCreateWindowSurface(Instance,
                                                               static_cast<GLFWwindow*>(Window->GetWindowRaw()),
                                                               nullptr,
                                                               reinterpret_cast<VkSurfaceKHR*>(&surface)));
    CRVM_LOG_RETURN_IF(res != vk::Result::eSuccess, nullptr, VulkanLogger, kError, "Failed to create surface.");
    return surface;
}

// ======================================================
// Conversions
// ======================================================

inline vk::Extent2D ToVulkanType(Size2D Size) {return {Size.X, Size.Y};}
inline vk::Extent3D ToVulkanType(Size3D Size) {return {Size.X, Size.Y, Size.Z};}

inline vk::ImageUsageFlags ToVulkanType(ImageUsageFlags Flags)
{
    vk::ImageUsageFlags outFlags{};
    if (Flags & ImageUsageFlagBits::ColorAttachment) outFlags |= vk::ImageUsageFlagBits::eColorAttachment;
    if (Flags & ImageUsageFlagBits::DepthStencilAttachment) outFlags |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
    if (Flags & ImageUsageFlagBits::ShaderResource) outFlags |= vk::ImageUsageFlagBits::eSampled;
    if (Flags & ImageUsageFlagBits::Storage) outFlags |= vk::ImageUsageFlagBits::eStorage;
    if (Flags & ImageUsageFlagBits::TransferDst) outFlags |= vk::ImageUsageFlagBits::eTransferDst;
    if (Flags & ImageUsageFlagBits::TransferSrc) outFlags |= vk::ImageUsageFlagBits::eTransferSrc;
    return outFlags;
}

inline vk::ShaderStageFlags ToVulkanType(ShaderStageFlags StageFlags)
{
    vk::ShaderStageFlags outFlags{};
    if (StageFlags & ShaderStageFlagBits::Vertex) outFlags |= vk::ShaderStageFlagBits::eVertex;
    if (StageFlags & ShaderStageFlagBits::Hull) outFlags |= vk::ShaderStageFlagBits::eTessellationControl;
    if (StageFlags & ShaderStageFlagBits::Domain) outFlags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
    if (StageFlags & ShaderStageFlagBits::Geometry) outFlags |= vk::ShaderStageFlagBits::eGeometry;
    if (StageFlags & ShaderStageFlagBits::Fragment) outFlags |= vk::ShaderStageFlagBits::eFragment;
    if (StageFlags & ShaderStageFlagBits::Mesh) outFlags |= vk::ShaderStageFlagBits::eMeshEXT;
    if (StageFlags & ShaderStageFlagBits::Task) outFlags |= vk::ShaderStageFlagBits::eTaskEXT;
    if (StageFlags & ShaderStageFlagBits::RtGen) outFlags |= vk::ShaderStageFlagBits::eRaygenKHR;
    if (StageFlags & ShaderStageFlagBits::RtIntersect) outFlags |= vk::ShaderStageFlagBits::eIntersectionKHR;
    if (StageFlags & ShaderStageFlagBits::RtAnyHit) outFlags |= vk::ShaderStageFlagBits::eAnyHitKHR;
    if (StageFlags & ShaderStageFlagBits::RtClosestHit) outFlags |= vk::ShaderStageFlagBits::eClosestHitKHR;
    if (StageFlags & ShaderStageFlagBits::RtMiss) outFlags |= vk::ShaderStageFlagBits::eMissKHR;
    if (StageFlags & ShaderStageFlagBits::RtCall) outFlags |= vk::ShaderStageFlagBits::eCallableKHR;
    if (StageFlags & ShaderStageFlagBits::Compute) outFlags |= vk::ShaderStageFlagBits::eCompute;
    return outFlags;
}

inline vk::ShaderStageFlagBits ToVulkanType(ShaderStageFlagBits StageFlagBit)
{
    switch (StageFlagBit)
    {
    case ShaderStageFlagBits::Vertex: return vk::ShaderStageFlagBits::eVertex;
    case ShaderStageFlagBits::Hull: return vk::ShaderStageFlagBits::eTessellationControl;
    case ShaderStageFlagBits::Domain: return vk::ShaderStageFlagBits::eTessellationEvaluation;
    case ShaderStageFlagBits::Geometry: return vk::ShaderStageFlagBits::eGeometry;
    case ShaderStageFlagBits::Fragment: return vk::ShaderStageFlagBits::eFragment;
    case ShaderStageFlagBits::Mesh: return vk::ShaderStageFlagBits::eMeshEXT;
    case ShaderStageFlagBits::Task: return vk::ShaderStageFlagBits::eTaskEXT;
    case ShaderStageFlagBits::RtGen: return vk::ShaderStageFlagBits::eRaygenKHR;
    case ShaderStageFlagBits::RtIntersect: return vk::ShaderStageFlagBits::eIntersectionKHR;
    case ShaderStageFlagBits::RtAnyHit: return vk::ShaderStageFlagBits::eAnyHitKHR;
    case ShaderStageFlagBits::RtClosestHit: return vk::ShaderStageFlagBits::eClosestHitKHR;
    case ShaderStageFlagBits::RtMiss: return vk::ShaderStageFlagBits::eMissKHR;
    case ShaderStageFlagBits::RtCall: return vk::ShaderStageFlagBits::eCallableKHR;
    case ShaderStageFlagBits::Compute: return vk::ShaderStageFlagBits::eCompute;
    default:
    {
        CRVM_ASSERT_MSG(false, "Unhandled ShaderStageFlagBit");
        return vk::ShaderStageFlagBits::eVertex;
    }
    }
}

inline vk::DescriptorType ToVulkanType(ShaderResourceType Type)
{
    switch (Type)
    {
    case ShaderResourceType::UniformBuffer:
        return vk::DescriptorType::eUniformBuffer;
    case ShaderResourceType::StorageBuffer:
        return vk::DescriptorType::eStorageBuffer;
    case ShaderResourceType::UniformTexelBuffer:
        return vk::DescriptorType::eUniformTexelBuffer;
    case ShaderResourceType::StorageTexelBuffer:
        return vk::DescriptorType::eStorageTexelBuffer;
    case ShaderResourceType::SampledTexture:
        return vk::DescriptorType::eSampledImage;
    case ShaderResourceType::StorageTexture:
        return vk::DescriptorType::eStorageImage;
    case ShaderResourceType::InlineUniformBlock:
        return vk::DescriptorType::eInlineUniformBlock;
    case ShaderResourceType::Sampler:
        return vk::DescriptorType::eSampler;
    case ShaderResourceType::CombinedTextureSampler:
        return vk::DescriptorType::eCombinedImageSampler;
    case ShaderResourceType::AccelerationStructure:
        return vk::DescriptorType::eAccelerationStructureKHR;
    default:
        break;
    }
    CRVM_ASSERT_MSG(false, "Unhandled ShaderResourceType");
    return vk::DescriptorType::eSampler;
}

inline vk::Format ToVulkanType(Format InFormat)
{
    static constexpr vk::Format kFormats[] {
        // S = 8
        // C = 1
        vk::Format::eR8Uint,
        vk::Format::eR8Sint,
        vk::Format::eR8Unorm,
        vk::Format::eR8Snorm,
        vk::Format::eR8Uscaled,
        vk::Format::eR8Sscaled,
        vk::Format::eUndefined,
        // C = 2
        vk::Format::eR8G8Uint,
        vk::Format::eR8G8Sint,
        vk::Format::eR8G8Unorm,
        vk::Format::eR8G8Snorm,
        vk::Format::eR8G8Uscaled,
        vk::Format::eR8G8Sscaled,
        vk::Format::eUndefined,
        // C = 3
        vk::Format::eR8G8B8Uint,
        vk::Format::eR8G8B8Sint,
        vk::Format::eR8G8B8Unorm,
        vk::Format::eR8G8B8Snorm,
        vk::Format::eR8G8B8Uscaled,
        vk::Format::eR8G8B8Sscaled,
        vk::Format::eUndefined,
        // C = 4
        vk::Format::eR8G8B8A8Uint,
        vk::Format::eR8G8B8A8Sint,
        vk::Format::eR8G8B8A8Unorm,
        vk::Format::eR8G8B8A8Snorm,
        vk::Format::eR8G8B8A8Uscaled,
        vk::Format::eR8G8B8A8Sscaled,
        vk::Format::eUndefined,

        // S = 16
        // C = 1
        vk::Format::eR16Uint,
        vk::Format::eR16Sint,
        vk::Format::eR16Unorm,
        vk::Format::eR16Snorm,
        vk::Format::eR16Uscaled,
        vk::Format::eR16Sscaled,
        vk::Format::eR16Sfloat,
        // C = 2
        vk::Format::eR16G16Uint,
        vk::Format::eR16G16Sint,
        vk::Format::eR16G16Unorm,
        vk::Format::eR16G16Snorm,
        vk::Format::eR16G16Uscaled,
        vk::Format::eR16G16Sscaled,
        vk::Format::eR16G16Sfloat,
        // C = 3
        vk::Format::eR16G16B16Uint,
        vk::Format::eR16G16B16Sint,
        vk::Format::eR16G16B16Unorm,
        vk::Format::eR16G16B16Snorm,
        vk::Format::eR16G16B16Uscaled,
        vk::Format::eR16G16B16Sscaled,
        vk::Format::eR16G16B16Sfloat,
        // C = 4
        vk::Format::eR16G16B16A16Uint,
        vk::Format::eR16G16B16A16Sint,
        vk::Format::eR16G16B16A16Unorm,
        vk::Format::eR16G16B16A16Snorm,
        vk::Format::eR16G16B16A16Uscaled,
        vk::Format::eR16G16B16A16Sscaled,
        vk::Format::eR16G16B16A16Sfloat,

        // S = 32
        // C = 1
        vk::Format::eR32Uint,
        vk::Format::eR32Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR32Sfloat,
        // C = 2
        vk::Format::eR32G32Uint,
        vk::Format::eR32G32Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR32G32Sfloat,
        // C = 3
        vk::Format::eR32G32B32Uint,
        vk::Format::eR32G32B32Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR32G32B32Sfloat,
        // C = 4
        vk::Format::eR32G32B32A32Uint,
        vk::Format::eR32G32B32A32Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR32G32B32A32Sfloat,

        // S = 64
        // C = 1
        vk::Format::eR64Uint,
        vk::Format::eR64Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR64Sfloat,
        // C = 2
        vk::Format::eR64G64Uint,
        vk::Format::eR64G64Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR64G64Sfloat,
        // C = 3
        vk::Format::eR64G64B64Uint,
        vk::Format::eR64G64B64Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR64G64B64Sfloat,
        // C = 4
        vk::Format::eR64G64B64A64Uint,
        vk::Format::eR64G64B64A64Sint,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eUndefined,
        vk::Format::eR64G64B64A64Sfloat
    };

    // indexing :
    // [0-28) (7 types * 4 counts) of 8 bit
    // [28-56) (7 types * 4 counts) of 16 bit
    // [56-68) (3 types * 4 counts) of 32 bit
    // [68-80) (3 types * 4 counts) of 32 bit

    static constexpr u8 kSizeBlockSize = 28;
    static constexpr u8 kCountBlockSize = 7;
    static constexpr u8 kTypeSizeDivisor = 8;

    const u8 index = (kSizeBlockSize * ((eastl::underlying_type_t<Format::ComponentSize>(InFormat.Size) - 1) / kTypeSizeDivisor)) // size block offset
             + (kCountBlockSize * (eastl::underlying_type_t<Format::ComponentCount>(InFormat.Count) - 1))
             + (eastl::underlying_type_t<Format::ScalarType>(InFormat.Type));
    return kFormats[index];
}

// ==================================================================
// Utility
// ==================================================================

inline vk::DeviceSize GetExtentTotal(vk::Extent2D Extent) { return Extent.width * Extent.height; }
inline vk::DeviceSize GetExtentTotal(vk::Extent3D Extent) { return Extent.width * Extent.height * Extent.depth; }
}
}
