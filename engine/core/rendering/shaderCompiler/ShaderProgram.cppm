// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "EASTL/vector.h"
export module ShaderCompiler:ShaderProgram;
import Common;
import :Types;

namespace Corvorium::ShaderCompiler {
export class ShaderProgram {
public:
    ShaderProgram() = default;
    ShaderProgram(const ShaderProgram& Other) = delete;
    ShaderProgram(ShaderProgram&& Other) noexcept = default;
    auto operator=(const ShaderProgram& Other) -> ShaderProgram& = delete;
    auto operator=(ShaderProgram&& Other) noexcept -> ShaderProgram& = default;

    ~ShaderProgram() = default;

    [[nodiscard]] const eastl::vector<u32>& GetByteCode() const { return ByteCode; }
    [[nodiscard]] const ShaderReflection& GetReflection() const { return ReflectionLayout; }

private:
    friend class CompileContext;
    eastl::vector<u32> ByteCode{};
    ShaderReflection ReflectionLayout{};
};
}
