// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

export module Common:Build;

export namespace Corvorium::Build {
enum class BuildMode {
    Release = 0,
    Development = 1,
    Debug = 2
};

constexpr auto DebugBuild() -> bool
{
    return static_cast<BuildMode>(CORVORIUM_BUILD_MODE) == BuildMode::Debug;
}

constexpr auto DevelopmentBuild() -> bool
{
    return static_cast<BuildMode>(CORVORIUM_BUILD_MODE) == BuildMode::Development;
}

constexpr auto ReleaseBuild() -> bool
{
    return static_cast<BuildMode>(CORVORIUM_BUILD_MODE) == BuildMode::Release;
}
}
