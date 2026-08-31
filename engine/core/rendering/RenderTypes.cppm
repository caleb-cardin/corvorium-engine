// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "common/FlagTypeMacro.hpp"
#include "EASTL/string.h"
export module Rendering:Types;
import RenderingCommon;
import Common;

namespace Corvorium::Rendering {
export enum class RenderError {
    Success = 0,
    InitializationFailed,
    AlreadyInitialized,
    InvalidHandle,

    // Framebuffer/swapchain
    WindowMinimized,
    SwapchainOOD,

    ResourceNotFound,

    ShaderCompilationFailure,
    InternalError
};

export template<class V>
using ResultValue = Error::ResultValueType<V, RenderError>;

using ResultError = Error::ResultErrorType<RenderError>;

export class PipelineCreateParams {};

export namespace Internal {
struct Shader {};

struct Buffer {};

struct Image {};

struct Mesh {};

struct Pipeline {};
}

export using ShaderHandle = HandleType<Internal::Shader>;
export using BufferHandle = HandleType<Internal::Buffer>;
export using ImageHandle = HandleType<Internal::Image>;
export using MeshHandle = HandleType<Internal::Mesh>;
export using PipelineHandle = HandleType<Internal::Pipeline>;

export struct ShaderStageRef {
    ShaderHandle Handle;
    const eastl::string& EntryPointName;
    ShaderStageFlagBits Stage;
};
}
