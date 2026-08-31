// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "common/Logging.hpp"

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

module ShaderCompiler:CompilerImpl;
import :Compiler;
import :ConversionHelpers;
import :Types;
import :Util;

namespace Corvorium::ShaderCompiler {
void ShaderCompiler::Initialize()
{
    if (!g_pGlobalSession)
    {
        constexpr SlangGlobalSessionDesc desc{};
        if (const auto result = slang::createGlobalSession(&desc, g_pGlobalSession.writeRef()); SLANG_FAILED(result))
        {
            CRVM_LOG(ShaderCompilerLogger, kError, "Failed to initialize global session.");
        }
    }
}

void ShaderCompiler::Shutdown()
{
    if (g_pGlobalSession)
    {
        g_pGlobalSession.setNull();
    }
}

ResultValue<CompileContext> ShaderCompiler::CreateCompileContext(const EnvironmentSettings& EnvSettings)
{
    slang::SessionDesc sessionDesc{};

    slang::TargetDesc targetDesc{};
    targetDesc.format = ToSlangType(EnvSettings.OutputTarget);
    targetDesc.profile = g_pGlobalSession->findProfile("spirv_1_5");
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    eastl::vector<slang::PreprocessorMacroDesc> preprocessMacros{EnvSettings.PreprocessorMacros.size()};
    for (u32 i = 0; const auto& macro: EnvSettings.PreprocessorMacros)
    {
        preprocessMacros[i].name = macro.name.c_str();
        preprocessMacros[i].value = macro.value.c_str();
        ++i;
    }

    eastl::vector<const char*> searchPaths(EnvSettings.SearchPaths.size());
    for (u32 i = 0; const auto& path: EnvSettings.SearchPaths)
    {
        searchPaths[i] = path.c_str();
        ++i;
    }
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = static_cast<SlangInt>(searchPaths.size());

    eastl::vector<slang::CompilerOptionEntry> compilerOptions;

    slang::CompilerOptionEntry& optimizationEntry = compilerOptions.emplace_back();
    optimizationEntry.name = slang::CompilerOptionName::Optimization;
    optimizationEntry.value.intValue0 = ToSlangType(EnvSettings.Optimization);

    sessionDesc.compilerOptionEntryCount = static_cast<u32>(compilerOptions.size());
    sessionDesc.compilerOptionEntries = compilerOptions.data();

    CompileContext context{};
    SlangResult res = g_pGlobalSession->createSession(sessionDesc, context.Session.writeRef());

    CRVM_LOG_RETURN_IF(SLANG_FAILED(res),
                       ResultError(ShaderCompilerError::InitializationError),
                       ShaderCompilerLogger,
                       kError,
                       "Failed to initialize shader compile context.")
    return context;
}
}
