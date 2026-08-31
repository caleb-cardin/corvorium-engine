// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"

export module ShaderCompiler:Compiler;
import :CompileContext;
import :Types;
import :Util;

namespace Corvorium::ShaderCompiler {
export class ShaderCompiler {
public:
    static void Initialize();
    static bool IsInitialized() { return !!g_pGlobalSession; }
    static void Shutdown();

    static ResultValue<CompileContext> CreateCompileContext(const EnvironmentSettings& EnvSettings);

private:
    inline static Slang::ComPtr<slang::IGlobalSession> g_pGlobalSession{nullptr};
};
}
