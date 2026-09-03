// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include <expected>
export module ResultValue;
export namespace Corvorium::Error {
template<typename V, typename E>
using ResultValueType = std::expected<V, E>;

template<typename E>
using ResultErrorType = std::unexpected<E>;
}
