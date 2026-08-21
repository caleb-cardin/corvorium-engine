// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include <expected>
export module Common:Result;
export namespace Corvorium {
template<typename V, typename E>
using ResultValueType = std::expected<V, E>;

template<typename E>
using ResultError = std::unexpected<E>;
}
