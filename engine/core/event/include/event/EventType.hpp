// Copyright © 2026 Caleb Cardin. All rights reserved.

#pragma once

#define EVENT_ID_HASH(_type) #_type##_hash

#define DECLARE_EVENT_TYPE_TRAITS(_type) \
    static constexpr bool IsEventType = true; \
    static constexpr Corvorium::Event::EventID EventId {EVENT_ID_HASH(_type)}
