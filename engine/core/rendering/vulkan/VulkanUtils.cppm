// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"
module Rendering:VulkanUtils;
import Common;
import RenderingCommon;
import OS;

namespace Corvorium::Rendering::Vulkan {
Log::Logger VulkanLogger{"VulkanRendering"};

auto& VulkanDispatcher = VULKAN_HPP_DEFAULT_DISPATCHER;

constexpr u32 kMaxFramesInFlight = 2;
constexpr u64 kMaxTimeout = UINT64_MAX;

namespace Util {
VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
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
    return VK_FALSE;
}

vk::SurfaceKHR CreateWindowSurface(const vk::Instance Instance, const OS::Window* Window)
{
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(Instance, static_cast<GLFWwindow*>(Window->GetWindowRaw()), nullptr, &surface);
    CRVM_LOG_RETURN_IF(res != VK_SUCCESS, nullptr, VulkanLogger, kError, "Failed to create surface.");
    return surface;
}

vk::ShaderStageFlags ToVulkanType(ShaderStageFlags StageFlags)
{
    vk::ShaderStageFlags OutFlags{};
    if (StageFlags & ShaderStageFlagBits::Vertex) OutFlags |= vk::ShaderStageFlagBits::eVertex;
    if (StageFlags & ShaderStageFlagBits::Hull) OutFlags |= vk::ShaderStageFlagBits::eTessellationControl;
    if (StageFlags & ShaderStageFlagBits::Domain) OutFlags |= vk::ShaderStageFlagBits::eTessellationEvaluation;
    if (StageFlags & ShaderStageFlagBits::Geometry) OutFlags |= vk::ShaderStageFlagBits::eGeometry;
    if (StageFlags & ShaderStageFlagBits::Fragment) OutFlags |= vk::ShaderStageFlagBits::eFragment;
    if (StageFlags & ShaderStageFlagBits::Mesh) OutFlags |= vk::ShaderStageFlagBits::eMeshEXT;
    if (StageFlags & ShaderStageFlagBits::Task) OutFlags |= vk::ShaderStageFlagBits::eTaskEXT;
    if (StageFlags & ShaderStageFlagBits::RtGen) OutFlags |= vk::ShaderStageFlagBits::eRaygenKHR;
    if (StageFlags & ShaderStageFlagBits::RtIntersect) OutFlags |= vk::ShaderStageFlagBits::eIntersectionKHR;
    if (StageFlags & ShaderStageFlagBits::RtAnyHit) OutFlags |= vk::ShaderStageFlagBits::eAnyHitKHR;
    if (StageFlags & ShaderStageFlagBits::RtClosestHit) OutFlags |= vk::ShaderStageFlagBits::eClosestHitKHR;
    if (StageFlags & ShaderStageFlagBits::RtMiss) OutFlags |= vk::ShaderStageFlagBits::eMissKHR;
    if (StageFlags & ShaderStageFlagBits::RtCall) OutFlags |= vk::ShaderStageFlagBits::eCallableKHR;
    if (StageFlags & ShaderStageFlagBits::Compute) OutFlags |= vk::ShaderStageFlagBits::eCompute;
    return OutFlags;
}

vk::DescriptorType ToVulkanType(ShaderResourceType Type)
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
}
}
