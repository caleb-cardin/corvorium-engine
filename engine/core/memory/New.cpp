// Copyright © 2026 Caleb Cardin. All rights reserved.
#include "CorvoriumApi.h"
#include "mimalloc.h"

#include "mimalloc-new-delete.h"

CRVM_API void* operator new[](size_t size, const char* name, int flags, unsigned debugFlags, const char* file, int line)
{
    return mi_malloc(size);
}

CRVM_API void* operator new[](size_t size,
                              size_t alignment,
                              size_t alignmentOffset,
                              const char* name,
                              int flags,
                              unsigned debugFlags,
                              const char* file,
                              int line)
{
    return mi_malloc_aligned(size, alignment);
}
