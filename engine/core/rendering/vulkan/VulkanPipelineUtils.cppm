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

namespace Corvorium::Rendering::Vulkan {
namespace {
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
constexpr vk::PipelineRasterizationStateCreateInfo kDefaultRasterState{};
constexpr vk::PipelineRasterizationStateCreateInfo kBackFaceCullRasterState{
    {},
    false,
    false,
    vk::PolygonMode::eFill,
    vk::CullModeFlagBits::eBack
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
constexpr vk::PipelineColorBlendAttachmentState kDefaultColorBlendAttachment{};
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
    enum class PrimitiveType {
        Default,
        PointList = Default,
        TriangleList
    };

    enum class DepthTestType {
        Default,
        Disabled = Default,
        Opaque,
        Transparent
    };

    enum class ColorBlendType {
        None,
        Alpha,
        Additive
    };

    struct ShaderStageInfo {
        vk::ShaderStageFlagBits Stage{};
        const char* EntryPoint{};
    };

    [[nodiscard]] bool IsComplete() const
    {
        if (!RenderingCreateInfo ||
            ShaderStages.empty() ||
            ColorBlends.empty() ||
            !Layout)
            return false;

        return true;
    }

    [[nodiscard]] const vk::PipelineRenderingCreateInfo* GetRenderingCreateInfo() const { return RenderingCreateInfo; }

    GraphicsPipelineDescBuilder& SetRenderingCreateInfo(vk::PipelineRenderingCreateInfo* Info)
    {
        RenderingCreateInfo = Info;
        return *this;
    }

    [[nodiscard]] const eastl::vector<vk::PipelineShaderStageCreateInfo>& GetShaderStages() const { return ShaderStages; }

    GraphicsPipelineDescBuilder& AddSingleShader(vk::ShaderModule Shader, vk::ShaderStageFlagBits Stage, const char* EntryPoint)
    {
        auto& NewStage = ShaderStages.emplace_back();
        NewStage.stage = Stage;
        NewStage.module = Shader;
        NewStage.pName = EntryPoint;
        return *this;
    }

    GraphicsPipelineDescBuilder& AddSingleShader(vk::ShaderModule Shader, const ShaderStageInfo& StageInfo)
    {
        return AddSingleShader(Shader, StageInfo.Stage, StageInfo.EntryPoint);
    }

    GraphicsPipelineDescBuilder& AddMultiShader(vk::ShaderModule Shader, eastl::span<ShaderStageInfo> StageInfos)
    {
        for (const auto& info: StageInfos)
        {
            AddSingleShader(Shader, info.Stage, info.EntryPoint);
        }
        return *this;
    }

    [[nodiscard]] vk::PipelineLayout GetLayout() const { return Layout; }

    GraphicsPipelineDescBuilder& SetLayout(vk::PipelineLayout InLayout)
    {
        Layout = InLayout;
        return *this;
    }

    [[nodiscard]] const eastl::vector<ColorBlendType>& GetColorBlends() const { return ColorBlends; }

    GraphicsPipelineDescBuilder& SetColorBlends(std::initializer_list<ColorBlendType>& initList)
    {
        std::construct_at(&ColorBlends, initList);
        return *this;
    }

    PrimitiveType Primitive{PrimitiveType::Default};
    DepthTestType DepthTest{DepthTestType::Default};
    bool BackfaceCullEnabled{false};

private:
    vk::PipelineRenderingCreateInfo* RenderingCreateInfo{nullptr};
    eastl::vector<vk::PipelineShaderStageCreateInfo> ShaderStages{};
    eastl::vector<ColorBlendType> ColorBlends{};
    vk::PipelineLayout Layout{nullptr};
};

bool MakeGraphicsPipelineCreateInfo(vk::GraphicsPipelineCreateInfo& Info, const GraphicsPipelineDescBuilder& Desc)
{
    CRVM_LOG_RETURN_IF(!Desc.IsComplete(),
                       false,
                       VulkanLogger,
                       kError,
                       "Failed to make create info: Graphics pipeline description was incomplete.");

    Info.pNext = Desc.GetRenderingCreateInfo();

    const auto& ShaderStages = Desc.GetShaderStages();
    Info.stageCount = static_cast<u32>(ShaderStages.size());
    Info.pStages = ShaderStages.data();

    switch (Desc.Primitive)
    {
    case GraphicsPipelineDescBuilder::PrimitiveType::PointList:
    {
        Info.pInputAssemblyState = &kDefaultIAState;
        break;
    }
    case GraphicsPipelineDescBuilder::PrimitiveType::TriangleList:
        Info.pInputAssemblyState = &kTriangleListIAState;
        break;
    }

    switch (Desc.DepthTest)
    {
    case GraphicsPipelineDescBuilder::DepthTestType::Disabled:
    {
        Info.pDepthStencilState = &kDefaultDepthStencilState;
        break;
    }
    case GraphicsPipelineDescBuilder::DepthTestType::Opaque:
    {
        Info.pDepthStencilState = &kOpaqueDepthStencilState;
        break;
    }
    case GraphicsPipelineDescBuilder::DepthTestType::Transparent:
    {
        Info.pDepthStencilState = &kTransparentDepthStencilState;
        break;
    }
    }

    Info.pTessellationState = &kDefaultTessellationState;
    Info.pViewportState = &kDefaultVPState;
    Info.pRasterizationState = Desc.BackfaceCullEnabled ? &kBackFaceCullRasterState : &kDefaultRasterState;
    Info.pMultisampleState = &kDefaultMSAAState;
    Info.pDynamicState = &kDefaultDynamicState;
    vk::PipelineColorBlendStateCreateInfo BlendState;
    BlendState.blendConstants.fill(1.f);
    eastl::vector<vk::PipelineColorBlendAttachmentState> blendAttachments;
    const auto& colorBlends = Desc.GetColorBlends();
    blendAttachments.resize(colorBlends.size());
    for (u32 i = 0; const auto& Blend: colorBlends)
    {
        switch (Blend)
        {
        case GraphicsPipelineDescBuilder::ColorBlendType::None:
        {
            blendAttachments[i] = kDefaultColorBlendAttachment;
            break;
        }
        case GraphicsPipelineDescBuilder::ColorBlendType::Alpha:
        {
            blendAttachments[i] = kAlphaColorBlendAttachment;
            break;
        }
        case GraphicsPipelineDescBuilder::ColorBlendType::Additive:
        {
            blendAttachments[i] = kAdditiveColorBlendAttachment;
            break;
        }
        }
        ++i;
    }

    Info.layout = Desc.GetLayout();
    return true;
}
}
