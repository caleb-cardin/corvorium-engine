// Copyright © 2026 Caleb Cardin. All rights reserved.
#pragma once

#ifdef CORVORIUM_DEBUG
#if defined(_MSC_VER)
#define CRVM_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__)
#define CRVM_DEBUG_BREAK() __builtin_debugtrap()
#elif defined(__GNUC__) && (__GNUC__ >= 11)
#define CRVM_DEBUG_BREAK() __builtin_debugtrap()
#else
#include <csignal>
#define CRVM_DEBUG_BREAK() raise(SIGTRAP)
#endif
#else
#define CRVM_DEBUG_BREAK()
#endif
