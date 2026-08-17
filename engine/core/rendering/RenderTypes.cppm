// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

export module Rendering:Types;
import Common;
export namespace Corvorium::Rendering {

enum class RenderError {
    Success = 0,
    InitializationError,
    ShaderCompilationFailure,
    InternalError
};
template<class V>
using ResultValue = ResultValueType<V, RenderError>;

}
