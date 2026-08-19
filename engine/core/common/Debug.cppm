// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "BreakpointDefinitions.hpp"
export module Common:Debug;
import :Build;
export namespace Corvorium {
CRVM_API void DebugBreak()
{
    if constexpr (Build::DebugBuild() || Build::DevelopmentBuild())
        CORVORIUM_DEBUG_BREAK();
}
}
