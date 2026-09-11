// Copyright © 2026 Caleb Cardin. All rights reserved.
#include "CorvoriumApi.h"
#include "mimalloc.h"

#include "mimalloc-new-delete.h"

CRVM_API void* operator new[](size_t size,
                              [[maybe_unused]] const char* name,
                              [[maybe_unused]] int flags,
                              [[maybe_unused]] unsigned debugFlags,
                              [[maybe_unused]] const char* file,
                              [[maybe_unused]] int line)
{
    return mi_malloc(size);
}

CRVM_API void* operator new[](size_t size,
                              size_t alignment,
                              [[maybe_unused]] size_t alignmentOffset,
                              [[maybe_unused]] const char* name,
                              [[maybe_unused]] int flags,
                              [[maybe_unused]] unsigned debugFlags,
                              [[maybe_unused]] const char* file,
                              [[maybe_unused]] int line)
{
    return mi_malloc_aligned(size, alignment);
}
