// Copyright © 2026 Caleb Cardin. All rights reserved.
#pragma once

#if defined(_MSC_VER)
#define CORVORIUM_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
#define CORVORIUM_DEBUG_BREAK() __builtin_debugtrap()
#elif defined(__GNUC__) && (__GNUC__ >= 11)
#define CORVORIUM_DEBUG_BREAK() __builtin_debugtrap()
#else
#include <csignal>
#define CORVORIUM_DEBUG_BREAK() raise(SIGTRAP)
#endif
