// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "EASTL/optional.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"
export module ShaderCompiler:Types;
import ResultValue;
import TypeDefs;
import RenderingCommon;

namespace Corvorium::ShaderCompiler {
export enum class ShaderCompilerError : u32 {
    Success,
    InitializationError,
    ShaderCompileFailure,
    ShaderReflectionFailure,
    InvalidShaderFile,
    InternalError
};

export template<typename T>
using ResultValue = Error::ResultValueType<T, ShaderCompilerError>;

export using ResultError = Error::ResultErrorType<ShaderCompilerError>;

export enum class OptimizationLevel {
    None,
    Default,
    High,
    Max
};

export enum class OutputIL : u8 {
    SPIRV,
    DXIL
};

export struct EnvironmentSettings {
    struct PreprocessorMacro {
        eastl::string name{};
        eastl::string value{};
    };

    OutputIL OutputTarget{OutputIL::SPIRV};
    OptimizationLevel Optimization{OptimizationLevel::Default};
    eastl::vector<eastl::string> SearchPaths{};
    eastl::vector<PreprocessorMacro> PreprocessorMacros{};
};

export const EnvironmentSettings kDefaultEnvironmentSettings{};

export enum class ShaderPipelineType : u8 {
    Unknown,
    Rasterization,
    Mesh,
    Raytracing,
    Compute
};

export struct ShaderReflection {
    ShaderReflection() = default;
    ShaderReflection(const ShaderReflection&) = delete;
    ShaderReflection& operator=(const ShaderReflection&) = delete;

    ShaderReflection(ShaderReflection&&) noexcept = default;
    ShaderReflection& operator=(ShaderReflection&&) noexcept = default;

    ~ShaderReflection() = default;

    struct EntryPoint {
        eastl::string Name;
        Rendering::ShaderStageFlagBits Stage;
        eastl::optional<eastl::vector<Rendering::VertexAttributeEntry>> VertexAttributes{};
    };

    ShaderPipelineType Type{ShaderPipelineType::Unknown};
    eastl::vector<EntryPoint> EntryPoints{};
    eastl::vector<Rendering::ShaderBindingEntry> BindingEntries{};
    eastl::vector<Rendering::ShaderPushConstantEntry> PushConstants{};
};
}
