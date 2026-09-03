// Copyright © 2026 Caleb Cardin. All rights reserved.
module;


#include "EASTL/array.h"
#include "EASTL/vector.h"
#include <vulkan/vulkan.hpp>

#include "common/Logging.hpp"
#include "EASTL/span.h"

module Rendering:VulkanPipelineUtils;
import Common;
import :VulkanUtils;
import :Types;

namespace Corvorium::Rendering::Vulkan {
namespace {
// Vertex
constexpr vk::PipelineVertexInputStateCreateInfo kDefaultVertexInput{
    {},
    0, nullptr,
    0, nullptr
};

// Input assembly
constexpr vk::PipelineInputAssemblyStateCreateInfo kDefaultIAState{};
constexpr vk::PipelineInputAssemblyStateCreateInfo kTriangleListIAState{
    {},
    vk::PrimitiveTopology::eTriangleList
};

// Tesselation
constexpr vk::PipelineTessellationStateCreateInfo kDefaultTessellationState{};

// Viewports
constexpr vk::PipelineViewportStateCreateInfo kDefaultVPState{
    {},
    1,
    nullptr,
    1,
    nullptr
};

// Rasterization
constexpr vk::PipelineRasterizationStateCreateInfo kDefaultRasterState{
    {},
    false,
    false,
    vk::PolygonMode::eFill,
    vk::CullModeFlagBits::eNone,
    vk::FrontFace::eCounterClockwise,
    false,
    0.f,
    0.f,
    0.f,
    1.f
};

// MSAA
constexpr vk::PipelineMultisampleStateCreateInfo kDefaultMSAAState{};

// DepthStencil
constexpr vk::PipelineDepthStencilStateCreateInfo kDefaultDepthStencilState{};
constexpr vk::PipelineDepthStencilStateCreateInfo kOpaqueDepthStencilState{
    {},
    true,
    true,
    vk::CompareOp::eLess,
};
constexpr vk::PipelineDepthStencilStateCreateInfo kTransparentDepthStencilState{
    {},
    true,
    false,
    vk::CompareOp::eLess,
};

// Color blend state
constexpr vk::PipelineColorBlendAttachmentState kDefaultColorBlendAttachment{
    false,
vk::BlendFactor::eZero,
    vk::BlendFactor::eZero,
    vk::BlendOp::eAdd,
vk::BlendFactor::eZero,
    vk::BlendFactor::eZero,
    vk::BlendOp::eAdd,
vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags
};
constexpr vk::PipelineColorBlendAttachmentState kAlphaColorBlendAttachment{
    true,
    vk::BlendFactor::eSrcAlpha,
    vk::BlendFactor::eOneMinusSrcAlpha,
    vk::BlendOp::eAdd,
    vk::BlendFactor::eOne,
    vk::BlendFactor::eOneMinusSrcAlpha,
    vk::BlendOp::eAdd,
    vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags
};
constexpr vk::PipelineColorBlendAttachmentState kAdditiveColorBlendAttachment{
    true,
    vk::BlendFactor::eSrcAlpha,
    vk::BlendFactor::eOne,
    vk::BlendOp::eAdd,
    vk::BlendFactor::eOne,
    vk::BlendFactor::eOne,
    vk::BlendOp::eAdd,
    vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags
};

// Dynamic state
constexpr eastl::array<vk::DynamicState, 2> kDefaultDynamicStates{
    vk::DynamicState::eViewport,
    vk::DynamicState::eScissor
};
constexpr vk::PipelineDynamicStateCreateInfo kDefaultDynamicState{
    {},
    2,
    kDefaultDynamicStates.data()
};
}

class GraphicsPipelineDescBuilder {
public:
    struct ShaderStageInfo {
        vk::ShaderStageFlagBits Stage{};
        const char* EntryPoint{};
    };

    [[nodiscard]] bool IsComplete() const;

    GraphicsPipelineDescBuilder& SetRenderingCreateInfo(vk::PipelineRenderingCreateInfo* Info);

    GraphicsPipelineDescBuilder& AddSingleShader(vk::ShaderModule Shader, vk::ShaderStageFlagBits Stage, const char* EntryPoint);
    GraphicsPipelineDescBuilder& AddSingleShader(vk::ShaderModule Shader, const ShaderStageInfo& StageInfo);
    GraphicsPipelineDescBuilder& AddMultiShader(vk::ShaderModule Shader, eastl::span<ShaderStageInfo> StageInfos);

    GraphicsPipelineDescBuilder& SetLayout(vk::PipelineLayout InLayout);

    GraphicsPipelineDescBuilder& SetColorBlends(ColorBlendType ColorBlend, u32 Count);

    GraphicsPipelineDescBuilder& SetPrimitiveType(PrimitiveType InType)
    {
        Primitive = InType;
        return *this;
    }

    GraphicsPipelineDescBuilder& SetDepthTestType(DepthTestType InType)
    {
        DepthTest = InType;
        return *this;
    }

    [[nodiscard]] ResultValue<vk::GraphicsPipelineCreateInfo> Build() const;

private:
    vk::PipelineRenderingCreateInfo* RenderingCreateInfo{nullptr};
    eastl::vector<vk::PipelineShaderStageCreateInfo> ShaderStages{};
    vk::PipelineLayout Layout{nullptr};
    eastl::vector<vk::PipelineColorBlendAttachmentState> BlendAttachments;
    vk::PipelineColorBlendStateCreateInfo BlendInfo{};
    PrimitiveType Primitive{PrimitiveType::Default};
    DepthTestType DepthTest{DepthTestType::Default};
};

bool GraphicsPipelineDescBuilder::IsComplete() const
{
    if (!RenderingCreateInfo ||
        ShaderStages.empty() ||
        !Layout)
        return false;

    return true;
}

GraphicsPipelineDescBuilder& GraphicsPipelineDescBuilder::SetRenderingCreateInfo(vk::PipelineRenderingCreateInfo* Info)
{
    RenderingCreateInfo = Info;
    return *this;
}

GraphicsPipelineDescBuilder& GraphicsPipelineDescBuilder::AddSingleShader(vk::ShaderModule Shader,
                                                                          vk::ShaderStageFlagBits Stage,
                                                                          const char* EntryPoint)
{
    auto& NewStage = ShaderStages.emplace_back();
    NewStage.stage = Stage;
    NewStage.module = Shader;
    NewStage.pName = EntryPoint;
    return *this;
}

GraphicsPipelineDescBuilder& GraphicsPipelineDescBuilder::AddSingleShader(vk::ShaderModule Shader, const ShaderStageInfo& StageInfo)
{
    return AddSingleShader(Shader, StageInfo.Stage, StageInfo.EntryPoint);
}

GraphicsPipelineDescBuilder& GraphicsPipelineDescBuilder::AddMultiShader(vk::ShaderModule Shader, eastl::span<ShaderStageInfo> StageInfos)
{
    for (const auto& info: StageInfos)
    {
        AddSingleShader(Shader, info.Stage, info.EntryPoint);
    }
    return *this;
}

GraphicsPipelineDescBuilder& GraphicsPipelineDescBuilder::SetLayout(vk::PipelineLayout InLayout)
{
    Layout = InLayout;
    return *this;
}

GraphicsPipelineDescBuilder& GraphicsPipelineDescBuilder::SetColorBlends(ColorBlendType ColorBlend, u32 Count)
{
    BlendAttachments.resize(Count);
    for (u32 i = 0; i < Count; ++i)
    {
        switch (ColorBlend)
        {
        case ColorBlendType::None:
        {
            BlendAttachments[i] = kDefaultColorBlendAttachment;
            break;
        }
        case ColorBlendType::Alpha:
        {
            BlendAttachments[i] = kAlphaColorBlendAttachment;
            break;
        }
        case ColorBlendType::Additive:
        {
            BlendAttachments[i] = kAdditiveColorBlendAttachment;
            break;
        }
        }
    }

    BlendInfo.attachmentCount = Count;
    BlendInfo.pAttachments = BlendAttachments.data();
    BlendInfo.blendConstants.fill(1.f);

    return *this;
}

ResultValue<vk::GraphicsPipelineCreateInfo> GraphicsPipelineDescBuilder::Build() const
{
    CRVM_LOG_RETURN_IF(!IsComplete(),
                       ResultError(RenderError::InitializationFailed),
                       VulkanLogger,
                       kError,
                       "Failed to make create info: Graphics pipeline description was incomplete.");

    vk::GraphicsPipelineCreateInfo info;

    info.pNext = RenderingCreateInfo;

    info.stageCount = static_cast<u32>(ShaderStages.size());
    info.pStages = ShaderStages.data();

    info.pVertexInputState = &kDefaultVertexInput;

    switch (Primitive)
    {
    case PrimitiveType::PointList:
    {
        info.pInputAssemblyState = &kDefaultIAState;
        break;
    }
    case PrimitiveType::TriangleList:
        info.pInputAssemblyState = &kTriangleListIAState;
        break;
    }

    switch (DepthTest)
    {
    case DepthTestType::Disabled:
    {
        info.pDepthStencilState = &kDefaultDepthStencilState;
        break;
    }
    case DepthTestType::Opaque:
    {
        info.pDepthStencilState = &kOpaqueDepthStencilState;
        break;
    }
    case DepthTestType::Transparent:
    {
        info.pDepthStencilState = &kTransparentDepthStencilState;
        break;
    }
    }

    info.pTessellationState = &kDefaultTessellationState;
    info.pViewportState = &kDefaultVPState;
    info.pRasterizationState = &kDefaultRasterState;
    info.pMultisampleState = &kDefaultMSAAState;
    info.pDynamicState = &kDefaultDynamicState;
    info.pColorBlendState = &BlendInfo;

    info.layout = Layout;
    return info;
}
}
