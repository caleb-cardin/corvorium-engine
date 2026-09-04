// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/Assert.hpp"
#include "EASTL/array.h"
#include "EASTL/functional.h"
#include "EASTL/optional.h"
#include "EASTL/vector.h"
#include "EASTL/vector_map.h"
export module Event:GlobalEventBus;
import TypeDefs;
import :Type;

namespace Corvorium::Event {
export enum class GlobalEventCategory : u8 {
    First,

    Last,
    LAYER_COUNT
};

export class GlobalEventBus;

export class EventRegisterTag {
public:
    EventRegisterTag() = default;
    EventRegisterTag(const EventRegisterTag& Other) = default;
    EventRegisterTag(EventRegisterTag&& Other) noexcept = default;
    auto operator=(const EventRegisterTag& Other) -> EventRegisterTag& = default;
    auto operator=(EventRegisterTag&& Other) noexcept -> EventRegisterTag& = default;
    ~EventRegisterTag() = default;

private:
    friend class GlobalEventBus;

    EventRegisterTag(const u64 Id, const u64 Index, const GlobalEventCategory Category)
        : Id{Id}
        , Index{Index}
        , Category{static_cast<std::underlying_type_t<GlobalEventCategory>>(Category)}
    {}

    u64 Id;
    u64 Index: 54;
    u64 Category: 8;
};

class GlobalEventBus {
    using Callback = eastl::function<bool(const void*)>;

    struct EventCallbackVector {
        eastl::vector<eastl::optional<Callback>> Data{};
        eastl::vector<u64> FreeList{};

        template<EventType E, typename Callable>
        u64 Emplace(Callable&& fn)
        {
            auto fnWrapper = [fn](const void* Event) -> bool {
                return fn(*static_cast<const E*>(Event));
            };

            if (!FreeList.empty())
            {
                const u64 nextAvailableIndex = FreeList.back();
                FreeList.pop_back();
                Data[nextAvailableIndex].emplace(eastl::move(fnWrapper));
                return nextAvailableIndex;
            }

            const u64 index = Data.size();
            Data.emplace_back(eastl::move(fnWrapper));
            return index;
        }

        void ClearAt(const u64 Index)
        {
            if (Index + 1 == Data.size())
            {
                Data.pop_back();
            }
            else
            {
                FreeList.push_back(Index);
                Data[Index].reset();
            }
        }
    };

    using LayeredEventCallbacks = eastl::array<
        EventCallbackVector,
        static_cast<std::underlying_type_t<GlobalEventCategory>>(GlobalEventCategory::LAYER_COUNT)
    >;

public:
    GlobalEventBus() = default;
    ~GlobalEventBus() = default;

    GlobalEventBus(const GlobalEventBus&) = delete;
    GlobalEventBus& operator=(const GlobalEventBus&) = delete;
    GlobalEventBus(GlobalEventBus&&) noexcept = default;
    GlobalEventBus& operator=(GlobalEventBus&&) noexcept = default;

    template<EventType E, typename Callable>
    [[nodiscard]] EventRegisterTag Subscribe(Callable&& CallbackFn, GlobalEventCategory Category)
    {
        CRVM_ASSERT(Category != GlobalEventCategory::LAYER_COUNT);
        constexpr u64 hashIndex = E::EventId;
        LayeredEventCallbacks* callbacks{nullptr};
        if (!CallbackLayerMap.contains(hashIndex))
        {
            auto [it, success] = CallbackLayerMap.insert(hashIndex);
            CRVM_ASSERT(success);
            callbacks = &it->second;
        }
        else
        {
            callbacks = &CallbackLayerMap[hashIndex];
        }

        const u64 categoryIndex = static_cast<u64>(Category);

        EventCallbackVector& cbVec = (*callbacks)[categoryIndex];

        const u64 index = cbVec.Emplace<E>(eastl::forward<Callable>(CallbackFn));

        return {hashIndex, index, Category};
    }

    void Unsubscribe(const EventRegisterTag& Tag)
    {
        EventCallbackVector& callbacks = CallbackLayerMap[Tag.Id][Tag.Category];
        callbacks.ClearAt(Tag.Index);
    }

    template<EventType E>
    void Signal(E Event)
    {
        constexpr u64 hashIndex = E::EventId;

        auto it = CallbackLayerMap.find(hashIndex);
        if (it == CallbackLayerMap.end()) return;

        LayeredEventCallbacks& callbacks = it->second;

        for (auto& layer: callbacks)
        {
            for (auto& cbOpt: layer.Data)
            {
                if (cbOpt.has_value())
                {
                    bool consumed = cbOpt.value()(&Event);
                    if (consumed) break;
                }
            }
        }
    }

private:
    eastl::vector_map<u64, LayeredEventCallbacks> CallbackLayerMap{};
};
}
