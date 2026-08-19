// Copyright © 2026 Caleb Cardin. All rights reserved.
#pragma once

#ifdef CORVORIUM_ENGINE_BUILD
#define CRVM_API __declspec(dllexport)
#else
#define CRVM_API __declspec(dllimport)
#endif
