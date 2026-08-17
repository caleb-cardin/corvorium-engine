// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "BreakpointDefinitions.hpp"

#include <cstdlib>

export module Common:Assert;
import :Build;
import :Types;
import :Logging;

namespace Corvorium {
static Log::Logger kAssertLogger{"Assert"};
}

export namespace Corvorium {

template<Detail::Boolean T>
bool Assert(T t)
{
    if constexpr (Build::DebugBuild()) assert(t);
    if constexpr (Build::DevelopmentBuild()) CORVORIUM_DEBUG_BREAK();
    if constexpr (Build::ReleaseBuild()) std::abort();
    return t;
}

}
