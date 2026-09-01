// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "shader/shared/SemanticNames.h"
#include "common/Assert.hpp"
#include "common/Logging.hpp"
#include "EASTL/array.h"

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

#include "EASTL/optional.h"
#include "EASTL/set.h"
#include "EASTL/sort.h"
#include "EASTL/string_hash_map.h"
module ShaderCompiler:CompileContextImpl;
import :CompileContext;
import :ConversionHelpers;
import :ShaderProgram;

namespace Corvorium::ShaderCompiler {
ResultValue<ShaderProgram> CompileContext::Compile(const eastl::string& File) const
{
    CRVM_ASSERT(Session.get());
    Slang::ComPtr<slang::IBlob> diagnostics;
    Slang::ComPtr<slang::IModule> module{Session->loadModule(File.c_str(), diagnostics.writeRef())};
    CRVM_LOG_RETURN_IF(!module,
                       ResultError(ShaderCompilerError::ShaderCompileFailure),
                       ShaderCompilerLogger,
                       kError,
                       "Failed to load shader module: {}",
                       diagnostics->getBufferPointer());

    i32 entryPointCount = module->getDefinedEntryPointCount();
    eastl::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints{};
    entryPoints.reserve(entryPointCount);
    for (i32 i = 0; i < entryPointCount; ++i)
    {
        Slang::ComPtr<slang::IEntryPoint> entry{nullptr};
        SlangResult res = module->getDefinedEntryPoint(i, entry.writeRef());
        CRVM_LOG_CONTINUE_IF(SLANG_FAILED(res), ShaderCompilerLogger, kError, "Failed to get defined shader entry point.");
        entryPoints.emplace_back(eastl::move(entry));
    }

    eastl::vector<slang::IComponentType*> components;
    components.emplace_back(module.get());
    for (const auto& entry: entryPoints)
    {
        components.emplace_back(static_cast<slang::IComponentType*>(entry.get()));
    }

    Slang::ComPtr<slang::IComponentType> composite{nullptr};
    SlangResult res = Session->createCompositeComponentType(
        components.data(),
        static_cast<SlangInt>(components.size()),
        composite.writeRef(),
        diagnostics.writeRef());
    CRVM_LOG_RETURN_IF(SLANG_FAILED(res),
                       ResultError(ShaderCompilerError::ShaderCompileFailure),
                       ShaderCompilerLogger,
                       kError,
                       "Failed to compose shader: {}",
                       diagnostics->getBufferPointer());

    Slang::ComPtr<slang::IComponentType> linkedProg{nullptr};
    res = composite->link(linkedProg.writeRef(), diagnostics.writeRef());
    CRVM_LOG_RETURN_IF(SLANG_FAILED(res),
                       ResultError(ShaderCompilerError::ShaderCompileFailure),
                       ShaderCompilerLogger,
                       kError,
                       "Failed to link shader program: {}",
                       diagnostics->getBufferPointer());

    Slang::ComPtr<slang::IBlob> byteCodeBlob{nullptr};
    res = linkedProg->getTargetCode(0, byteCodeBlob.writeRef(), diagnostics.writeRef());
    CRVM_LOG_RETURN_IF(SLANG_FAILED(res),
                       ResultError(ShaderCompilerError::ShaderCompileFailure),
                       ShaderCompilerLogger,
                       kError,
                       "Failed to get shader byte code: {}",
                       diagnostics->getBufferPointer());

    ShaderProgram shader;
    shader.ByteCode.resize(byteCodeBlob->getBufferSize());
    std::memcpy(shader.ByteCode.data(), byteCodeBlob->getBufferPointer(), byteCodeBlob->getBufferSize());

    ResultValue<ShaderReflection> result = Reflect(linkedProg);
    CRVM_LOG_RETURN_IF(!result.has_value(), ResultError(result.error()), ShaderCompilerLogger, kError, "Shader reflection failed.");
    shader.ReflectionLayout = eastl::move(result.value());


    return shader;
}

using TypeKind = slang::TypeReflection::Kind;

static void ProcessParameterReflection(ShaderReflection& Reflection,
                                       eastl::string_hash_map<Rendering::ShaderBindingEntry*>& LeafVariables,
                                       slang::VariableLayoutReflection* VariableLayout,
                                       eastl::optional<u32> InheritedSet = {})
{
    slang::TypeLayoutReflection* typeLayout = VariableLayout->getTypeLayout();
    const TypeKind typeKind = typeLayout->getKind();
    if (typeKind == TypeKind::ParameterBlock || typeKind == TypeKind::Struct)
    {
        eastl::optional<u32> set = typeKind == TypeKind::ParameterBlock
                                       ? VariableLayout->getBindingSpace(VariableLayout->getCategory())
                                       : InheritedSet;
        u32 fieldCount = typeLayout->getFieldCount();
        for (u32 i = 0; i < fieldCount; ++i)
        {
            ProcessParameterReflection(Reflection, LeafVariables, typeLayout->getFieldByIndex(i), set);
        }
    }
    else
    {
        Rendering::ShaderBindingEntry& bindingEntry = Reflection.BindingEntries.emplace_back();
        bindingEntry.Name = VariableLayout->getName();
        bindingEntry.Type = FromSlangType(typeLayout->getBindingRangeType(0));
        bindingEntry.Set = InheritedSet.has_value()
                               ? InheritedSet.value()
                               : static_cast<u32>(VariableLayout->getBindingSpace(VariableLayout->getCategory()));
        bindingEntry.Binding = VariableLayout->getBindingIndex();
        SlangInt bindingCount = typeLayout->getBindingRangeBindingCount(0);
        bindingEntry.Count = bindingCount == SLANG_UNBOUNDED_SIZE ? eastl::optional<u32>{} : static_cast<u32>(bindingCount);
        LeafVariables.insert(VariableLayout->getName(), &bindingEntry);
    }
}

static void ProcessPushConstantReflection(ShaderReflection& Reflection,
                                          eastl::string_hash_map<Rendering::ShaderPushConstantEntry*>& PushVariables,
                                          slang::VariableLayoutReflection* VariableLayout)
{
    Rendering::ShaderPushConstantEntry& pushConstantEntry = Reflection.PushConstants.emplace_back();
    pushConstantEntry.Name = VariableLayout->getName();
    pushConstantEntry.Offset = static_cast<u32>(VariableLayout->getOffset());
    slang::TypeLayoutReflection* typeLayout = VariableLayout->getTypeLayout();
    pushConstantEntry.Size = static_cast<u32>(typeLayout->getSize(slang::ParameterCategory::PushConstantBuffer));
    PushVariables.insert(VariableLayout->getName(), &pushConstantEntry);
}

static void ProcessShaderStageFlags(eastl::string_hash_map<Rendering::ShaderBindingEntry*>& BindingMap,
                                    eastl::string_hash_map<Rendering::ShaderPushConstantEntry*>& PushConstantMap,
                                    Rendering::ShaderStageFlagBits Stage,
                                    slang::VariableLayoutReflection* VariableLayout)
{
    slang::ParameterCategory category = VariableLayout->getCategory();

    if (category == slang::ParameterCategory::VaryingInput || category == slang::ParameterCategory::VaryingOutput)
    {
        return;
    }

    if (category != slang::ParameterCategory::PushConstantBuffer)
    {
        slang::TypeLayoutReflection* typeLayout = VariableLayout->getTypeLayout();
        const TypeKind typeKind = typeLayout->getKind();
        if (typeKind == TypeKind::ParameterBlock || typeKind == TypeKind::Struct)
        {
            u32 fieldCount = typeLayout->getFieldCount();
            for (u32 i = 0; i < fieldCount; ++i)
            {
                ProcessShaderStageFlags(BindingMap, PushConstantMap, Stage, typeLayout->getFieldByIndex(i));
            }
        }
        else
        {
            const char* name = VariableLayout->getName();
            if (BindingMap.contains(name))
            {
                BindingMap[name]->Stage |= Stage;
            }
        }
    }
    else
    {
        const char* name = VariableLayout->getName();
        if (PushConstantMap.contains(name))
        {
            PushConstantMap[name]->Stage |= Stage;
        }
    }
}

using TypeScalar = slang::TypeReflection::ScalarType;

static [[nodiscard]] ShaderCompilerError ProcessVertexAttributes(eastl::vector<Rendering::VertexAttributeEntry>& ReflectionAttributes,
                                                                 slang::VariableLayoutReflection* VariableLayout)
{
    slang::ParameterCategory category = VariableLayout->getCategory();
    if (category != slang::ParameterCategory::VaryingInput)
    {
        return ShaderCompilerError::Success;
    }

    Rendering::VertexAttributeEntry& vertexAttribute = ReflectionAttributes.emplace_back();
    vertexAttribute.Location = VariableLayout->getBindingIndex();

    slang::TypeLayoutReflection* typeLayout = VariableLayout->getTypeLayout();
    vertexAttribute.Format.ComponentCount = static_cast<u8>(typeLayout->getRowCount());
    vertexAttribute.Format.ComponentSize = static_cast<u8>(typeLayout->getColumnCount());
    vertexAttribute.Format.Type = FromSlangType(typeLayout->getScalarType());

    eastl::string_view semName = VariableLayout->getSemanticName();
    if (semName == SHADER_SEMANTIC_NAME_POSITION) vertexAttribute.Semantic = VertexAttributeEntry::SemanticType::Position;
    else if (semName == SHADER_SEMANTIC_NAME_NORMAL) vertexAttribute.Semantic = VertexAttributeEntry::SemanticType::Normal;
    else if (semName == SHADER_SEMANTIC_NAME_TEXCOORD) vertexAttribute.Semantic = VertexAttributeEntry::SemanticType::TextureCoord;
    else if (semName == SHADER_SEMANTIC_NAME_COLOR) vertexAttribute.Semantic = VertexAttributeEntry::SemanticType::Color;
    else
    {
        CRVM_LOG(ShaderCompilerLogger, kError, "Shader contained undefined semantic vertex attribute.");
        return ShaderCompilerError::ShaderCompileFailure;
    }

    vertexAttribute.SemanticIndex = static_cast<u32>(VariableLayout->getSemanticIndex());

    return ShaderCompilerError::Success;
}

ResultValue<ShaderReflection> CompileContext::Reflect(Slang::ComPtr<slang::IComponentType>& LinkedProgram) const
{
    Slang::ComPtr<slang::IBlob> diagnostics{};
    slang::ProgramLayout* programLayout = LinkedProgram->getLayout(0, diagnostics.writeRef());
    CRVM_LOG_RETURN_IF(!programLayout,
                       ResultError(ShaderCompilerError::ShaderCompileFailure),
                       ShaderCompilerLogger,
                       kError,
                       "Failed to get shader layout: {}",
                       diagnostics->getBufferPointer());

    u32 parameterCount = programLayout->getParameterCount();

    ShaderReflection Reflect{};

    Reflect.Type = ValidateShaderFileAsPipeline(programLayout);
    CRVM_LOG_RETURN_IF(Reflect.Type == ShaderPipelineType::Unknown,
                       ResultError(ShaderCompilerError::InvalidShaderFile),
                       ShaderCompilerLogger,
                       kError,
                       "Shader file-as-pipeline validation failed!"); {
        eastl::string_hash_map<Rendering::ShaderBindingEntry*> bindingVariables;
        eastl::string_hash_map<Rendering::ShaderPushConstantEntry*> pushConstantVariables;
        for (u32 i = 0; i < parameterCount; ++i)
        {
            slang::VariableLayoutReflection* varLayout = programLayout->getParameterByIndex(i);
            slang::ParameterCategory category = varLayout->getCategory();
            if (category != slang::ParameterCategory::PushConstantBuffer)
            {
                if (category == slang::ParameterCategory::VaryingInput || category == slang::ParameterCategory::VaryingOutput)
                {
                    continue;
                }
                ProcessParameterReflection(Reflect, bindingVariables, varLayout);
            }
            else
            {
                ProcessPushConstantReflection(Reflect, pushConstantVariables, varLayout);
            }
        }

        u32 entryPointCount = static_cast<u32>(programLayout->getEntryPointCount());
        for (u32 i = 0; i < entryPointCount; ++i)
        {
            ShaderReflection::EntryPoint& newEntryPoint = Reflect.EntryPoints.emplace_back();
            slang::EntryPointReflection* entryPoint = programLayout->getEntryPointByIndex(i);
            u32 entryParameterCount = entryPoint->getParameterCount();
            newEntryPoint.Stage = FromSlangType(entryPoint->getStage());

            // Loop through params
            for (u32 j = 0; j < entryParameterCount; ++j)
            {
                slang::VariableLayoutReflection* varLayout = entryPoint->getParameterByIndex(i);
                ProcessShaderStageFlags(bindingVariables, pushConstantVariables, newEntryPoint.Stage, varLayout);

                if (newEntryPoint.Stage == Rendering::ShaderStageFlagBits::Vertex)
                {
                    newEntryPoint.VertexAttributes.emplace();
                    const ShaderCompilerError err = ProcessVertexAttributes(newEntryPoint.VertexAttributes.value(), varLayout);
                    if (err != ShaderCompilerError::Success)
                    {
                        return ResultError(err);
                    }
                }
            }
        }
    }

    eastl::sort(Reflect.BindingEntries.begin(),
                Reflect.BindingEntries.end(),
                [](const Rendering::ShaderBindingEntry& lhs, const Rendering::ShaderBindingEntry& rhs) {
                    return lhs.Set < rhs.Set;
                });

    return Reflect;
}

ShaderPipelineType CompileContext::ValidateShaderFileAsPipeline(slang::ProgramLayout* ProgramLayout)
{
    static constexpr ShaderStageFlags RasterExclusiveFlags{
        ShaderStageFlagBits::Vertex |
        ShaderStageFlagBits::Domain |
        ShaderStageFlagBits::Hull |
        ShaderStageFlagBits::Geometry
    };
    static constexpr ShaderStageFlags MeshExclusiveFlags{
        ShaderStageFlagBits::Mesh |
        ShaderStageFlagBits::Task
    };
    static constexpr ShaderStageFlags RaytracingExclusiveFlags{
        ShaderStageFlagBits::RtGen |
        ShaderStageFlagBits::RtIntersect |
        ShaderStageFlagBits::RtAnyHit |
        ShaderStageFlagBits::RtClosestHit |
        ShaderStageFlagBits::RtMiss |
        ShaderStageFlagBits::RtCall
    };
    static constexpr ShaderStageFlags ComputeExclusiveFlags{
        ShaderStageFlagBits::Compute
    };

    // Note: ordering of the elements of the following arrays needs to remain respective to each other.

    static constexpr eastl::array<const ShaderStageFlags*, 4> ExclusiveFlags{
        &RasterExclusiveFlags,
        &MeshExclusiveFlags,
        &RaytracingExclusiveFlags,
        &ComputeExclusiveFlags,
    };
    static constexpr eastl::array<ShaderPipelineType, 4> Types{
        ShaderPipelineType::Rasterization,
        ShaderPipelineType::Mesh,
        ShaderPipelineType::Rasterization,
        ShaderPipelineType::Compute
    };

    ShaderPipelineType predictedType{ShaderPipelineType::Unknown};
    ShaderStageFlags stages{ShaderStageFlagBits::None};

    SlangUInt entryPointCount = ProgramLayout->getEntryPointCount();
    eastl::multiset<ShaderStageFlagBits> stageBitSet{};
    for (SlangUInt i = 0; i < entryPointCount; ++i)
    {
        slang::EntryPointReflection* entryPoint = ProgramLayout->getEntryPointByIndex(i);
        ShaderStageFlagBits thisStage = FromSlangType(entryPoint->getStage());
        stageBitSet.insert(thisStage);
        stages |= thisStage;
    }

    for (u8 i = 0; const auto* exclusiveFlags: ExclusiveFlags)
    {
        if (predictedType == ShaderPipelineType::Unknown)
        {
            if (stages & *exclusiveFlags)
            {
                predictedType = Types[i];
            }
        }
        else if (stages & *exclusiveFlags)
        {
            return ShaderPipelineType::Unknown;
        }
        ++i;
    }

    if (predictedType == ShaderPipelineType::Unknown)
    {
        // The only way this will be reached is if the shader file ONLY contains a fragment shader,
        // as this is the only flag not exclusively grouped. Still an invalid file.
        return ShaderPipelineType::Unknown;
    }
    else if (predictedType == ShaderPipelineType::Rasterization)
    {
        // Checking required counts
        if (stageBitSet.count(ShaderStageFlagBits::Vertex) != 1 || //required
            stageBitSet.count(ShaderStageFlagBits::Fragment) != 1 || //required
            stageBitSet.count(ShaderStageFlagBits::Geometry) > 1) //optional
        {
            return ShaderPipelineType::Unknown;
        }
        // If either domain or hull, count of both must be one
        if (stages & ShaderStageFlagBits::Domain || stages & ShaderStageFlagBits::Hull)
        {
            if (stageBitSet.count(ShaderStageFlagBits::Domain) != 1 || //required
                stageBitSet.count(ShaderStageFlagBits::Hull) != 1) //required
            {
                return ShaderPipelineType::Unknown;
            }
        }
    }
    else if (predictedType == ShaderPipelineType::Mesh)
    {
        if (stageBitSet.count(ShaderStageFlagBits::Mesh) != 1 || //required
            stageBitSet.count(ShaderStageFlagBits::Fragment) != 1 || //required
            stageBitSet.count(ShaderStageFlagBits::Task) > 1) //optional
        {
            return ShaderPipelineType::Unknown;
        }
    }
    else if (predictedType == ShaderPipelineType::Raytracing)
    {
        // Note: This assumes the count of other RT shaders is correct, since simple count checking isn't a robust way to check for this.
        if (stageBitSet.count(ShaderStageFlagBits::RtGen) < 1 || //required
            stageBitSet.count(ShaderStageFlagBits::Fragment) != 0) //invalid
        {
            return ShaderPipelineType::Unknown;
        }
    }
    else if (predictedType == ShaderPipelineType::Compute)
    {
        if (stageBitSet.count(ShaderStageFlagBits::Compute) != 1 || //required
            stageBitSet.count(ShaderStageFlagBits::Fragment) != 0) //invalid
        {
            return ShaderPipelineType::Unknown;
        }
    }

    return predictedType;
}
}
