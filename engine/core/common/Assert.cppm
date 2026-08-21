// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "CorvoriumApi.h"
#include "BreakpointDefinitions.hpp"

#include <cassert>
#include <cstdlib>

export module Common:Assert;
import :Build;
import :Logging;

namespace Corvorium {
static Log::Logger AssertLogger{"Assert"};
}

export namespace Corvorium {
template<Detail::Boolean T>
CRVM_API bool Assert(T t)
{
    if constexpr (Build::DebugBuild())
        assert(t);
    if constexpr (Build::DevelopmentBuild())
        CORVORIUM_DEBUG_BREAK();
    if constexpr (Build::ReleaseBuild())
        std::abort();
    return t;
}
}
