// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/atomic.h"
#include  "EASTL/functional.h"

export module Common:Types;
import :TypeDefs;

export namespace Corvorium {
template<typename T>
class HandleType {
public:
    using Handle_t = u64;
    static constexpr Handle_t kInvalidHandle = ~0x0;

    HandleType()
        : Handle{kInvalidHandle}
    {}
    ~HandleType() = default;

    HandleType(const HandleType&) = default;
    HandleType(HandleType&&) = default;
    HandleType& operator=(const HandleType&) = default;
    HandleType& operator=(HandleType&&) = default;

    static HandleType New()
    {
        return HandleType(NextHandle.fetch_add(1ull, eastl::memory_order_relaxed));
    }

    bool operator==(const HandleType& Other) const
    {
        return Handle == Other.Handle;
    }

private:
    friend class eastl::hash<Corvorium::HandleType<T>>;
    static eastl::atomic<u64> NextHandle;
    u64 Handle;

    explicit HandleType(const Handle_t NewHandle)
        : Handle{NewHandle}
    {}
};

template<typename T>
eastl::atomic<u64> HandleType<T>::NextHandle{0};
}

export namespace eastl {
template<typename T>
struct hash<Corvorium::HandleType<T>> : public hash<uint64_t> {
    size_t operator()(const Corvorium::HandleType<T> HandleType)
    {
        return hash<uint64_t>::operator()(HandleType.Handle);
    }
};
}
