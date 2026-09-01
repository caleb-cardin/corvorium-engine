// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "EASTL/string.h"
#include "EASTL/vector.h"

#include "slang/slang.h"
#include "slang/slang-com-ptr.h"
export module ShaderCompiler:CompileContext;
import :Util;
import :Types;
import :ShaderProgram;

namespace Corvorium::ShaderCompiler {
export class CompileContext {
public:
    CompileContext() = default;
    CompileContext(const CompileContext&) = delete;
    CompileContext(CompileContext&&) noexcept = default;
    CompileContext& operator=(const CompileContext&) = delete;
    CompileContext& operator=(CompileContext&&) noexcept = default;
    ~CompileContext() = default;

    [[nodiscard]] ResultValue<ShaderProgram> Compile(const eastl::string& File) const;

private:
    friend class ShaderCompiler;

    Slang::ComPtr<slang::ISession> Session{nullptr};

    [[nodiscard]] ResultValue<ShaderReflection> Reflect(Slang::ComPtr<slang::IComponentType>& LinkedProgram) const;
    static [[nodiscard]] ShaderPipelineType ValidateShaderFileAsPipeline(slang::ProgramLayout* ProgramLayout);
};
}
