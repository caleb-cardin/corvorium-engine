// Copyright © 2026 Caleb Cardin. All rights reserved.

#pragma once

#include "DebugBreak.hpp"

#include <cassert>

#ifdef CORVORIUM_DEBUG
#define CRVM_ASSERT(_cond) assert(_cond)
#define CRVM_ASSERT_MSG(_cond, _msg) assert(_cond && _msg)
#define CRVM_VERIFY(_cond) assert(_cond)
#define CRVM_VERIFY_MSG(_cond, _msg) assert(_cond && _msg)
#else
#define CRVM_ASSERT(_cond)
#define CRVM_ASSERT_MSG(_cond, _msg)
#define CRVM_VERIFY(_cond) _cond
#define CRVM_VERIFY_MSG(_cond, _msg) _cond
#endif
