// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Logging.hpp"

#include "eastl/string.h"
#include "eastl/vector.h"
#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

#include <ranges>
module Rendering:ShaderCompiler;
import :Shader;
import :Types;
import Common;

namespace Corvorium::Rendering::ShaderCompiler {
static Log::Logger ShaderCompileLogger{"Shader Compiler"};

slang::EntryPointReflection* GetEntryPointReflection(const Slang::ComPtr<slang::IEntryPoint>& pEntryPoint);

class CompilationEnvironment;
class CompilationModule;
class ShaderCompiler;

enum class CompilerOutputType {
    SPIRV,
    DXIL
};

enum class ShaderFileType {
    Auto,
    Single,
    Multi
};

struct ShaderEntryPointNames {
    struct Entry {
        ShaderStageFlagBits stage{};
        eastl::string name{};
    };

    eastl::vector<Entry> names{};
};


class CompilationModule {
public:
    CompilationModule() = delete;
    ~CompilationModule() = default;

    CompilationModule(CompilationModule&&) noexcept = default;
    CompilationModule& operator=(CompilationModule&&) noexcept = default;

    CompilationModule(const CompilationModule&) = default;
    CompilationModule& operator=(const CompilationModule&) = default;

    [[nodiscard]] ResultValue<eastl::vector<u32>> CompileShaderByStage(ShaderStageFlagBits shaderStage) const;
    [[nodiscard]] ResultValue<eastl::vector<u32>> CompileShaderMultiEntry(ShaderEntryPointNames& entryPointNames);

private:
    Slang::ComPtr<slang::IModule> m_pModule;
    Slang::ComPtr<slang::ISession> m_pSession;
    Slang::ComPtr<slang::IComponentType> m_pProgram;

    friend class CompilationEnvironment;

    CompilationModule(Slang::ComPtr<slang::ISession> pSession, Slang::ComPtr<slang::IModule>&& module);

    [[nodiscard]] ResultValue<u32> GetShaderEntryPoint(
        ShaderStageFlagBits shaderStage,
        slang::IEntryPoint** ppEntryPtr
    ) const;

    [[nodiscard]] slang::IModule* GetModule() const;
    [[nodiscard]] slang::ISession* GetSession() const;
};

struct EnvironmentSettings {
    struct PreprocessorMacro {
        eastl::string name{};
        eastl::string value{};
    };

    enum class OptimizationLevel {
        None,
        Default,
        High,
        Max
    };

    eastl::vector<CompilerOutputType> outputTargets{CompilerOutputType::SPIRV};
    eastl::vector<eastl::string> searchPaths{};
    eastl::vector<PreprocessorMacro> preprocessorMacros{};

    // TODO : Compiler options;
    OptimizationLevel optimization = OptimizationLevel::Default;
};

class CompilationEnvironment {
public:
    CompilationEnvironment() = default;
    ~CompilationEnvironment();

    CompilationEnvironment(const CompilationEnvironment&) = delete;
    auto operator=(const CompilationEnvironment&) -> CompilationEnvironment& = delete;

    [[nodiscard]] ResultValue<CompilationModule> CreateShaderModule(const eastl::string& filePath) const;

private:
    friend class ShaderCompiler;
    friend class CompilationModule;
    explicit CompilationEnvironment(Slang::ComPtr<slang::ISession>&& session);

    Slang::ComPtr<slang::ISession> m_pSession;
};

class ShaderCompiler {
public:
    static auto Initialize() -> void;
    static auto IsInitialized() -> bool;
    static auto Shutdown() -> void;

    static auto CreateCompileEnvironment(const EnvironmentSettings& settings = {}) -> CompilationEnvironment;

private:
    static Slang::ComPtr<slang::IGlobalSession> g_pGlobalSession;
};

Slang::ComPtr<slang::IGlobalSession> ShaderCompiler::g_pGlobalSession{nullptr};

static void LogShaderCompilerDiagnostics(slang::IBlob* diagnostics)
{
    if (diagnostics == nullptr)
    {
        return;
    }

    CRVM_LOG(ShaderCompileLogger, kError, "Slang diagnostics: {}", static_cast<const char*>(diagnostics->getBufferPointer()));
}

static SlangStage StageToSlangStage(const ShaderStageFlagBits stage)
{
    switch (stage)
    {
    case ShaderStageFlagBits::Vertex:
        return SLANG_STAGE_VERTEX;
    case ShaderStageFlagBits::TessCtrl:
        return SLANG_STAGE_HULL;
    case ShaderStageFlagBits::TessEval:
        return SLANG_STAGE_DOMAIN;
    case ShaderStageFlagBits::Geometry:
        return SLANG_STAGE_GEOMETRY;
    case ShaderStageFlagBits::Fragment:
        return SLANG_STAGE_FRAGMENT;
    case ShaderStageFlagBits::Comp:
        return SLANG_STAGE_COMPUTE;
    case ShaderStageFlagBits::RtGen:
        return SLANG_STAGE_RAY_GENERATION;
    case ShaderStageFlagBits::RtAnyHit:
        return SLANG_STAGE_ANY_HIT;
    case ShaderStageFlagBits::RtClosestHit:
        return SLANG_STAGE_CLOSEST_HIT;
    case ShaderStageFlagBits::RtMiss:
        return SLANG_STAGE_MISS;
    case ShaderStageFlagBits::RtIntersect:
        return SLANG_STAGE_INTERSECTION;
    case ShaderStageFlagBits::RtCall:
        return SLANG_STAGE_CALLABLE;
    case ShaderStageFlagBits::Task:
        return SLANG_STAGE_AMPLIFICATION;
    case ShaderStageFlagBits::Mesh:
        return SLANG_STAGE_MESH;
    case ShaderStageFlagBits::None:
        [[fallthrough]];
    default:
    {
        return SLANG_STAGE_NONE;
    }
    }
}

static ShaderStageFlagBits SlangToStageFlagBits(const SlangStage slangStage)
{
    switch (slangStage)
    {
    case SLANG_STAGE_VERTEX:
        return ShaderStageFlagBits::Vertex;
    case SLANG_STAGE_HULL:
        return ShaderStageFlagBits::TessCtrl;
    case SLANG_STAGE_DOMAIN:
        return ShaderStageFlagBits::TessEval;
    case SLANG_STAGE_GEOMETRY:
        return ShaderStageFlagBits::Geometry;
    case SLANG_STAGE_FRAGMENT:
        return ShaderStageFlagBits::Fragment;
    case SLANG_STAGE_COMPUTE:
        return ShaderStageFlagBits::Comp;
    case SLANG_STAGE_RAY_GENERATION:
        return ShaderStageFlagBits::RtGen;
    case SLANG_STAGE_ANY_HIT:
        return ShaderStageFlagBits::RtAnyHit;
    case SLANG_STAGE_CLOSEST_HIT:
        return ShaderStageFlagBits::RtClosestHit;
    case SLANG_STAGE_MISS:
        return ShaderStageFlagBits::RtMiss;
    case SLANG_STAGE_INTERSECTION:
        return ShaderStageFlagBits::RtIntersect;
    case SLANG_STAGE_CALLABLE:
        return ShaderStageFlagBits::RtCall;
    case SLANG_STAGE_AMPLIFICATION:
        return ShaderStageFlagBits::Task;
    case SLANG_STAGE_MESH:
        return ShaderStageFlagBits::Mesh;
    default:
        return ShaderStageFlagBits::None;
    }
}

// ====================================================================
//  class CompilationModule
// ====================================================================

slang::EntryPointReflection* GetEntryPointReflection(const Slang::ComPtr<slang::IEntryPoint>& pEntryPoint)
{
    if (auto* entryLayout = pEntryPoint->getLayout(); entryLayout->getEntryPointCount())
    {
        return entryLayout->getEntryPointByIndex(0);
    }
    return nullptr;
}

ResultValue<eastl::vector<u32>> CompilationModule::CompileShaderByStage(const ShaderStageFlagBits shaderStage) const
{
    Slang::ComPtr<slang::IEntryPoint> pEntryPoint;
    if (const auto entryIndexResult = GetShaderEntryPoint(shaderStage, pEntryPoint.writeRef()); entryIndexResult.has_value())
    {
        Slang::ComPtr<slang::IBlob> pDiagnostics;
        if (!entryIndexResult.has_value())
        {
            return ResultError(RenderError::InternalError);
        }
        const auto index = entryIndexResult.value();
        Slang::ComPtr<slang::IBlob> binary;
        auto result = pEntryPoint->getEntryPointCode(index, 0, binary.writeRef(), pDiagnostics.writeRef());
        if (!SLANG_SUCCEEDED(result))
        {
            LogShaderCompilerDiagnostics(pDiagnostics);
            return ResultError(RenderError::InternalError);
        }
        const auto pCode = static_cast<const u32*>(binary->getBufferPointer());
        return eastl::vector<u32>{pCode, pCode + (binary->getBufferSize() / sizeof(u32))};
    }
    return ResultError(RenderError::InternalError);
}

ResultValue<eastl::vector<u32>> CompilationModule::CompileShaderMultiEntry(ShaderEntryPointNames& entryPointNames)
{
    i32 count = m_pModule->getDefinedEntryPointCount();
    eastl::vector<Slang::ComPtr<slang::IEntryPoint>> entryPoints(count);
    eastl::vector<slang::IComponentType*> components(count + 1);
    components[0] = m_pModule.get();
    for (i32 i = 0; i < count; ++i)
    {
        [[maybe_unused]] auto result = m_pModule->getDefinedEntryPoint(i, entryPoints[i].writeRef());
        components[i + 1] = std::move(entryPoints[i].get());
        entryPointNames.names.emplace_back(
            std::move(SlangToStageFlagBits(GetEntryPointReflection(entryPoints[i])->getStage())),
            std::move(entryPoints[i]->getFunctionReflection()->getName())
        );
    }
    Slang::ComPtr<slang::IComponentType> pProgram{};
    m_pSession->createCompositeComponentType(components.data(), components.size(), pProgram.writeRef());

    Slang::ComPtr<slang::IComponentType> linkedProgram;
    Slang::ComPtr<slang::IBlob> pDiagnostics;
    auto result = pProgram->link(linkedProgram.writeRef(), pDiagnostics.writeRef());
    if (!SLANG_SUCCEEDED(result))
    {
        LogShaderCompilerDiagnostics(pDiagnostics);
        return ResultError(RenderError::ShaderCompilationFailure);
    }

    Slang::ComPtr<slang::IBlob> binary;
    result = pProgram->getTargetCode(0, binary.writeRef(), pDiagnostics.writeRef());
    if (!SLANG_SUCCEEDED(result))
    {
        LogShaderCompilerDiagnostics(pDiagnostics);
        return ResultError(RenderError::ShaderCompilationFailure);
    }
    const auto pCode = static_cast<const u32*>(binary->getBufferPointer());
    return eastl::vector<u32>{pCode, pCode + (binary->getBufferSize() / sizeof(u32))};
}

CompilationModule::CompilationModule(Slang::ComPtr<slang::ISession> pSession, Slang::ComPtr<slang::IModule>&& module)
    : m_pModule{std::move(module)}
    , m_pSession{std::move(pSession)}
{
    eastl::vector<Slang::ComPtr<slang::IComponentType>> linkComponents;
    eastl::vector<slang::IComponentType*> rawLinkComponents;
    const i32 definedEntryPointCount = m_pModule->getDefinedEntryPointCount();
    for (i32 entryPointIndex = 0; entryPointIndex < definedEntryPointCount; ++entryPointIndex)
    {
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        const auto result = m_pModule->getDefinedEntryPoint(entryPointIndex, entryPoint.writeRef());
        CRVM_LOG_RETURN_IF(!SLANG_SUCCEEDED(result),
                           /*void*/
                           ;,
                           ShaderCompileLogger,
                           kError,
                           "Error acquiring defined entry point.");
        auto& newComponent = linkComponents.emplace_back(entryPoint.get());
        rawLinkComponents.emplace_back(newComponent.get());
    }

    Slang::ComPtr<slang::IComponentType> pComposedComponent;
    Slang::ComPtr<slang::IBlob> pDiagnosticsBlob;
    auto result = m_pSession->createCompositeComponentType(
        static_cast<slang::IComponentType* const*>(rawLinkComponents.data()),
        rawLinkComponents.size(),
        pComposedComponent.writeRef(),
        pDiagnosticsBlob.writeRef()
    );
    if (!SLANG_SUCCEEDED(result))
    {
        LogShaderCompilerDiagnostics(pDiagnosticsBlob.get());
    }

    result = pComposedComponent->link(m_pProgram.writeRef(), pDiagnosticsBlob.writeRef());
    if (!SLANG_SUCCEEDED(result))
    {
        LogShaderCompilerDiagnostics(pDiagnosticsBlob.get());
    }
}

auto CompilationModule::GetShaderEntryPoint(
    const ShaderStageFlagBits shaderStage,
    slang::IEntryPoint** ppEntryPtr
) const -> ResultValue<u32>
{
    Assert(!!m_pModule);
    Assert(!!ppEntryPtr);
    const auto slangStage = StageToSlangStage(shaderStage);
    Slang::ComPtr<slang::IEntryPoint> pEntryPoint;
    const auto entryPointCount = m_pModule->getDefinedEntryPointCount();
    for (i32 i = 0; i < entryPointCount; ++i)
    {
        auto result = m_pModule->getDefinedEntryPoint(i, pEntryPoint.writeRef());
        if (SLANG_FAILED(result)) continue;
        auto* entryLayout = pEntryPoint->getLayout();
        const auto count = entryLayout->getEntryPointCount();
        for (u32 j = 0; j < count; ++j)
        {
            if (auto* entryRefl = entryLayout->getEntryPointByIndex(j); entryRefl && entryRefl->getStage() == slangStage)
            {
                *ppEntryPtr = pEntryPoint.get();
                return j;
            }
        }
    }
    return ResultError(RenderError::InitializationError);
}

auto CompilationModule::GetModule() const -> slang::IModule*
{
    return m_pModule.get();
}

auto CompilationModule::GetSession() const -> slang::ISession*
{
    return m_pSession.get();
}


// ====================================================================
//  class CompilationEnvironment
// ====================================================================

CompilationEnvironment::~CompilationEnvironment() = default;

ResultValue<CompilationModule> CompilationEnvironment::CreateShaderModule(const eastl::string& filePath) const
{
    Slang::ComPtr<slang::IModule> pModule{nullptr};
    // Slang shader module;
    {
        Slang::ComPtr<slang::IBlob> pDiagnosticsBlob;
        pModule = m_pSession->loadModule(filePath.c_str(), pDiagnosticsBlob.writeRef());
        LogShaderCompilerDiagnostics(pDiagnosticsBlob);
        if (!pModule)
        {
            return ResultError(RenderError::InitializationError);
        }
    }
    return CompilationModule{m_pSession, std::move(pModule)};
}

CompilationEnvironment::CompilationEnvironment(Slang::ComPtr<slang::ISession>&& session)
    : m_pSession{session}
{}

// ====================================================================
//  class ShaderCompiler
// ====================================================================

void ShaderCompiler::Initialize()
{
    if (!g_pGlobalSession)
    {
        constexpr SlangGlobalSessionDesc desc{};
        if (const auto result = slang::createGlobalSession(&desc, g_pGlobalSession.writeRef()); SLANG_FAILED(result))
        {
            CRVM_LOG(ShaderCompileLogger, kError, "Failed to initialize global session.");
        }
    }
}

bool ShaderCompiler::IsInitialized()
{
    return !!g_pGlobalSession;
}

void ShaderCompiler::Shutdown()
{
    g_pGlobalSession.setNull();
}

CompilationEnvironment ShaderCompiler::CreateCompileEnvironment(const EnvironmentSettings& settings)
{
    slang::SessionDesc sessionDescription{};

    // Output targets
    eastl::vector<slang::TargetDesc> targets(settings.outputTargets.size());
    for (auto [target, settingTarget]: std::views::zip(targets, settings.outputTargets))
    {
        switch (settingTarget)
        {
        case CompilerOutputType::SPIRV:
        {
            target.format = SLANG_SPIRV;
            target.profile = g_pGlobalSession->findProfile("spirv_1_5");
            break;
        }
        case CompilerOutputType::DXIL:
        {
            target.format = SLANG_DXIL;
            target.profile = g_pGlobalSession->findProfile("spirv_1_5");
            break;
        }
        default:
        {
            CRVM_LOG(ShaderCompileLogger, kError, "Unhandled shader compiler target.");
            break;
        }
        }
    }
    sessionDescription.targetCount = static_cast<SlangInt>(targets.size());
    sessionDescription.targets = targets.data();

    // Preprocessor definitions
    eastl::vector<slang::PreprocessorMacroDesc> preprocessorMacros(settings.preprocessorMacros.size());
    for (auto [str, cStr]: std::views::zip(settings.preprocessorMacros, preprocessorMacros))
    {
        cStr.name = str.name.c_str();
        cStr.value = str.value.c_str();
    }
    sessionDescription.preprocessorMacros = preprocessorMacros.data();
    sessionDescription.preprocessorMacroCount = static_cast<SlangInt>(preprocessorMacros.size());

    // Search Paths (__import, #include)
    eastl::vector<const char*> searchPaths(settings.searchPaths.size());
    for (auto [str, cStr]: std::views::zip(settings.searchPaths, searchPaths))
    {
        cStr = str.c_str();
    }
    sessionDescription.searchPaths = searchPaths.data();
    sessionDescription.searchPathCount = static_cast<SlangInt>(searchPaths.size());

    // Compiler Options
    // Optimization
    slang::CompilerOptionEntry optimizationEntry{};
    optimizationEntry.name = slang::CompilerOptionName::Optimization;
    switch (settings.optimization)
    {
    case EnvironmentSettings::OptimizationLevel::None:
    {
        optimizationEntry.value.intValue0 = SLANG_OPTIMIZATION_LEVEL_NONE;
        break;
    }
    case EnvironmentSettings::OptimizationLevel::Default:
    {
        optimizationEntry.value.intValue0 = SLANG_OPTIMIZATION_LEVEL_DEFAULT;
        break;
    }
    case EnvironmentSettings::OptimizationLevel::High:
    {
        optimizationEntry.value.intValue0 = SLANG_OPTIMIZATION_LEVEL_HIGH;
        break;
    }
    case EnvironmentSettings::OptimizationLevel::Max:
    {
        optimizationEntry.value.intValue0 = SLANG_OPTIMIZATION_LEVEL_MAXIMAL;
        break;
    }
    default:
        break;
    }
    sessionDescription.compilerOptionEntryCount = 1;
    sessionDescription.compilerOptionEntries = &optimizationEntry;

    Slang::ComPtr<slang::ISession> compilerSession{};
    g_pGlobalSession->createSession(sessionDescription, compilerSession.writeRef());
    return CompilationEnvironment{std::move(compilerSession)};
}
}
