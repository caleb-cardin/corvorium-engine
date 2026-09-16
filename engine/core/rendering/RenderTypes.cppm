// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"

#include "common/FlagTypeMacro.hpp"
#include "EASTL/string.h"
export module Rendering:Types;
import RenderingCommon;
import ResultValue;
import CommonTypes;

namespace Corvorium::Rendering {
export enum class RenderError {
    Success = 0,
    InitializationFailed,
    AlreadyInitialized,
    InvalidParameters,
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

export enum class PrimitiveType : u8 {
    Default,
    PointList,
    TriangleList = Default
};

export enum class DepthTestType : u8 {
    Default,
    Disabled = Default,
    Opaque,
    Transparent
};

export enum class ColorBlendType : u8 {
    None,
    Alpha,
    Additive
};

export struct GraphicsPipelineCreateParams {
    ShaderHandle Module;
    PrimitiveType Primitive;
    DepthTestType DepthTest;
    ColorBlendType ColorBlend;
};
}
