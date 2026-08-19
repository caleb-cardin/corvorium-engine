// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
export module Common:Build;

export namespace Corvorium::Build {
CRVM_API enum class BuildMode {
    Release = 0,
    Development = 1,
    Debug = 2
};

consteval auto DebugBuild() -> bool
{
    return static_cast<BuildMode>(CORVORIUM_BUILD_MODE) == BuildMode::Debug;
}

consteval auto DevelopmentBuild() -> bool
{
    return static_cast<BuildMode>(CORVORIUM_BUILD_MODE) == BuildMode::Development;
}

consteval auto ReleaseBuild() -> bool
{
    return static_cast<BuildMode>(CORVORIUM_BUILD_MODE) == BuildMode::Release;
}
}
