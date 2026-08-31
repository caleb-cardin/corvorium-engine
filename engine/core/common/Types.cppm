// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "EASTL/atomic.h"
#include "EASTL/functional.h"
#include "EASTL/sort.h"
#include "EASTL/vector.h"

export module Common:Types;
import :TypeDefs;

namespace Corvorium {
export template<typename T>
class HandleType {
public:
    using Type = T;
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

    bool operator<(const HandleType& Other) const { return Handle < Other.Handle; }

    bool operator<(const Handle_t Other) const { return Handle < Other; }

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

export struct MultiHandleHash {
    template<typename T>
    size_t operator()(const eastl::vector<HandleType<T>>& Handles)
    {
        eastl::vector<HandleType<T>> sorted = Handles;
        eastl::sort(sorted.begin(),
                    sorted.end(),
                    [](const HandleType<T>& Lhs, const HandleType<T>& Rhs) {
                        return Lhs < Rhs;
                    });
        size_t hash = 0;
        for (const auto& handle: sorted)
        {
            hash ^= eastl::hash<uint64_t>()(handle.Get()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};


template<typename T>
eastl::atomic<u64> HandleType<T>::NextHandle{0};
}
