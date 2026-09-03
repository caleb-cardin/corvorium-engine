// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "include/common/DebugBreak.hpp"
export module Debug;
import Build;
export namespace Corvorium {
CRVM_API void DebugBreak()
{
    if constexpr (Build::DebugBuild() || Build::DevelopmentBuild())
        CRVM_DEBUG_BREAK();
}
}
