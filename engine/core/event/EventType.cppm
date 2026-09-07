// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

export module Event:Type;
export import TypeDefs;
export import Hash;
import std;

namespace Corvorium::Event {
export using EventID = u64;

static constexpr u64 kMaxEventStructSize = 256;

export template<typename T>
concept EventType = T::IsEventType && std::is_trivially_copyable_v<T> && (sizeof(T) <= kMaxEventStructSize);
}
