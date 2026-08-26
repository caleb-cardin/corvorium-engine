// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/atomic.h"
#include "EASTL/functional.h"

export module Common:Types;
import :TypeDefs;

namespace Corvorium {
export template<typename T>
class HandleType {
public:
    using Handle_t = u64;
    static constexpr Handle_t kInvalidHandle = ~0x0ull;

    HandleType() = default;

    ~HandleType() = default;

    HandleType(const HandleType&) = default;
    HandleType(HandleType&&) = default;
    HandleType& operator=(const HandleType&) = default;
    HandleType& operator=(HandleType&&) = default;

    static HandleType New()
    {
        return HandleType(NextHandle.fetch_add(1ull, eastl::internal::memory_order_relaxed_s{}));
    }

    bool operator==(const HandleType& Other) const { return Handle == Other.Handle; }

    bool operator==(const Handle_t Other) const { return Handle == Other; }

    [[nodiscard]] u64 Get() const { return Handle; }

private:
    static eastl::atomic<u64> NextHandle;
    u64 Handle{};

    explicit HandleType(const Handle_t NewHandle)
        : Handle{NewHandle}
    {}
};

export struct HandleHash {
    template<typename T>
    size_t operator()(const HandleType<T>& Handle) const
    {
        return eastl::hash<uint64_t>()(Handle.Get());
    }
};


template<typename T>
eastl::atomic<u64> HandleType<T>::NextHandle{0};
}
