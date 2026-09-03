// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "common/Logging.hpp"

#include "EASTL/string_hash_map.h"
#include "EASTL/optional.h"
#include "EASTL/sort.h"
#include "EASTL/span.h"
#include "EASTL/unique_ptr.h"
#include "EASTL/vector.h"

#include "vulkan/vulkan.hpp"
module Rendering:VulkanShaderCache;
import RenderingCommon;
import ShaderCompiler;
import OS;
import :Types;
import :VulkanUtils;

/* Slang shader files are considered to always consists of set of shaders composing complete pipelines.
 * TODO: Enforce and check slang file -> complete pipeline
 */

namespace Corvorium::Rendering::Vulkan {
struct VulkanShader {
    struct DescriptorBinding {
        u32 Set;
        u32 Binding;
        eastl::optional<u32> Size;
    };

    struct EntryPoint {
        eastl::string Name;
        ShaderStageFlagBits Stage;
    };

    ShaderModuleHandle Handle;
    eastl::vector<EntryPoint> EntryPoints{};
    eastl::optional<eastl::vector<Rendering::VertexAttributeEntry>> VertexAttributes{};
    vk::ShaderModule Module{nullptr};
    vk::PipelineLayout Layout{nullptr};
    eastl::string_hash_map<DescriptorBinding> BindingMap{};
    eastl::vector<vk::DescriptorSetLayout> DescriptorLayouts{};
    eastl::vector<vk::PushConstantRange> PushConstantRanges{};
};

class VulkanShaderCache {
    using DescriptorBindingKey = eastl::vector;

public:
    VulkanShaderCache() = delete;
    VulkanShaderCache(const VulkanShaderCache&) = delete;
    VulkanShaderCache(VulkanShaderCache&&) = delete;
    VulkanShaderCache& operator=(const VulkanShaderCache&) noexcept = default;
    VulkanShaderCache& operator=(VulkanShaderCache&&) noexcept = default;
    ~VulkanShaderCache();

    VulkanShaderCache(vk::Device InDevice, FileSystem::IFileProvider* InFileProvider);

    [[nodiscard]] ShaderCompiler::CompileContext* GetDefaultCompileContext() const { return DefaultCompileContext.get(); }

    ResultValue<const VulkanShader*> GetOrCreateShader(const eastl::string& ShaderPath,
                                                       const ShaderCompiler::CompileContext* Context = nullptr);
    const VulkanShader* GetShader(ShaderModuleHandle Shader);

private:
    vk::Device Device{nullptr};
    FileSystem::IFileProvider* FileProvider{nullptr};
    eastl::unique_ptr<ShaderCompiler::CompileContext> DefaultCompileContext{nullptr};

    eastl::string_hash_map<ShaderModuleHandle> FileToHandleMap{};
    eastl::hash_map<ShaderModuleHandle, VulkanShader, HandleHash> HandleToShaderMap{};

    void CreateDescriptorData(const eastl::vector<Rendering::ShaderBindingEntry>& Bindings,
                              eastl::vector<vk::DescriptorSetLayout>& OutLayouts);

    void TeardownShader(VulkanShader& Shader);
};

VulkanShaderCache::~VulkanShaderCache()
{
    for (auto& entry: HandleToShaderMap)
    {
        TeardownShader(entry.second);
    }
}

VulkanShaderCache::VulkanShaderCache(vk::Device InDevice,
                                     FileSystem::IFileProvider* InFileProvider)
    : Device{InDevice}
    , FileProvider{InFileProvider}
    , DefaultCompileContext{}
{
    namespace SC = ShaderCompiler;

    SC::ShaderCompiler::Initialize();

    CRVM_ASSERT(Device);
    CRVM_ASSERT(FileProvider);
    CRVM_ASSERT(SC::ShaderCompiler::IsInitialized());

    SC::ResultValue<SC::CompileContext> result = SC::ShaderCompiler::CreateCompileContext(SC::kDefaultEnvironmentSettings);
    CRVM_ASSERT(result.has_value());
    DefaultCompileContext = eastl::make_unique<SC::CompileContext>(eastl::move(result.value()));
}

ResultValue<const VulkanShader*> VulkanShaderCache::GetOrCreateShader(const eastl::string& ShaderPath,
                                                                      const ShaderCompiler::CompileContext* CompileContext)
{
    if (FileToHandleMap.contains(ShaderPath.c_str()))
    {
        ShaderModuleHandle handle = FileToHandleMap[ShaderPath.c_str()];
        CRVM_ASSERT(handle);
        return GetShader(handle);
    }

    if (!CompileContext)
    {
        CompileContext = GetDefaultCompileContext();
    }

    namespace SC = ShaderCompiler;
    FileSystem::ResultValue<eastl::string> fileResult = FileProvider->ResolveVirtualPath(ShaderPath);
    CRVM_LOG_RETURN_IF(!fileResult.has_value(), ResultError(RenderError::ResourceNotFound), VulkanLogger, kError, "Failed to find shader.");
    SC::ResultValue<SC::ShaderProgram> result = CompileContext->Compile(fileResult.value());
    CRVM_LOG_RETURN_IF(!fileResult.has_value(),
                       ResultError(RenderError::ShaderCompilationFailure),
                       VulkanLogger,
                       kError,
                       "Failed to compile shader.");

    SC::ShaderProgram& program = result.value();

    vk::ShaderModuleCreateInfo shaderInfo;
    shaderInfo.codeSize = static_cast<size_t>(program.GetByteCode().size());
    shaderInfo.pCode = program.GetByteCode().data();

    vk::ResultValue<vk::ShaderModule> moduleResult = Device.createShaderModule(shaderInfo);
    CRVM_LOG_RETURN_IF(!fileResult.has_value(),
                       ResultError(RenderError::ShaderCompilationFailure),
                       VulkanLogger,
                       kError,
                       "Failed to create shader module.");

    VulkanShader shader{};
    shader.Handle = ShaderModuleHandle::New();
    shader.Module = moduleResult.value;

    for (const auto& entry: program.GetReflection().EntryPoints)
    {
        shader.EntryPoints.emplace_back(VulkanShader::EntryPoint{
            .Name = entry.Name,
            .Stage = entry.Stage
        });
        if (entry.VertexAttributes.has_value())
        {
            const auto& vertexAttributes = entry.VertexAttributes.value();
            shader.VertexAttributes.emplace(vertexAttributes);
        }
    }

    CreateDescriptorData(program.GetReflection().BindingEntries, shader.DescriptorLayouts);

    shader.PushConstantRanges.reserve(program.GetReflection().PushConstants.size());
    for (const auto& pushConstant: program.GetReflection().PushConstants)
    {
        shader.PushConstantRanges.emplace_back(Util::ToVulkanType(pushConstant.Stage), pushConstant.Offset, pushConstant.Size);
    }

    vk::PipelineLayoutCreateInfo info;
    info.setLayoutCount = static_cast<u32>(shader.DescriptorLayouts.size());
    info.pSetLayouts = shader.DescriptorLayouts.data();
    info.pushConstantRangeCount = static_cast<u32>(shader.PushConstantRanges.size());
    info.pPushConstantRanges = shader.PushConstantRanges.data();

    vk::ResultValue pipelineLayoutResult = Device.createPipelineLayout(info);
    if (!result.has_value())
    {
        TeardownShader(shader);
        return ResultError(RenderError::InitializationFailed);
    }
    shader.Layout = pipelineLayoutResult.value;

    const auto [it, success] = HandleToShaderMap.emplace(shader.Handle, eastl::move(shader));
    CRVM_ASSERT(success);
    return &HandleToShaderMap[it->first];
}

const VulkanShader* VulkanShaderCache::GetShader(ShaderModuleHandle Shader)
{
    if (HandleToShaderMap.contains(Shader))
    {
        return &HandleToShaderMap[Shader];
    }
    return nullptr;
}

void VulkanShaderCache::CreateDescriptorData(const eastl::vector<Rendering::ShaderBindingEntry>& Bindings,
                                             eastl::vector<vk::DescriptorSetLayout>& OutLayouts)
{
    if (Bindings.empty()) return;
    u32 setCount = Bindings.back().Set + 1;

    eastl::vector<eastl::vector<vk::DescriptorSetLayoutBinding>> bindings{};
    bindings.resize(setCount);
    for (const auto& bindingInfo: Bindings)
    {
        auto& layoutBindings = bindings[bindingInfo.Set];

        vk::DescriptorSetLayoutBinding& newBinding = layoutBindings.emplace_back();
        newBinding.binding = bindingInfo.Binding;
        // TODO: bindless descriptor maximum
        CRVM_ASSERT_MSG(bindingInfo.Count.has_value(), "Unbounded shader resource binding count unsupported.");
        newBinding.descriptorCount = bindingInfo.Count.value();
        newBinding.descriptorType = Util::ToVulkanType(bindingInfo.Type);
        newBinding.stageFlags = Util::ToVulkanType(bindingInfo.Stage);
    }

    for (u32 setIndex = 0; setIndex < setCount; ++setIndex)
    {
        vk::DescriptorSetLayoutCreateInfo setLayoutInfo;
        setLayoutInfo.bindingCount = static_cast<u32>(bindings[setIndex].size());
        setLayoutInfo.pBindings = bindings[setIndex].data();

        auto result = Device.createDescriptorSetLayout(setLayoutInfo);
        CRVM_LOG_RETURN_IF(!result.has_value(), /*void*/;, VulkanLogger, kError, "Failed to create shader descriptors.");
        OutLayouts.emplace_back(result.value);
    }
}

void VulkanShaderCache::TeardownShader(VulkanShader& Shader)
{
    if (Shader.Module) Device.destroyShaderModule(Shader.Module);
    if (Shader.Layout) Device.destroyPipelineLayout(Shader.Layout);
    for (const auto descriptorLayout: Shader.DescriptorLayouts) Device.destroyDescriptorSetLayout(descriptorLayout);
    Shader.PushConstantRanges.clear();
}
}
