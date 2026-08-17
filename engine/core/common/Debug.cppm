// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "BreakpointDefinitions.hpp"
export module Common:Debug;
import :Build;
export namespace Corvorium {

void DebugBreak()
{
    if constexpr (Build::DebugBuild() || Build::DevelopmentBuild()) CORVORIUM_DEBUG_BREAK();
}

}
