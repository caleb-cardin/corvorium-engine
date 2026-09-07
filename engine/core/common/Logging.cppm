// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include "CorvoriumApi.h"
#include "common/FlagTypeMacro.hpp"

#include "spdlog/logger.h"
#include "EASTL/string.h"
#include "EASTL/unique_ptr.h"

export module Logging;
export import TypeDefs;
import Build;
import Debug;
import Flags;
import std;

namespace Corvorium {
export enum class LogLevelBits : u8 {
    Unknown = 0,
    Info = 0x1 << 1,
    Warning = 0x1 << 2,
    Error = 0x1 << 3,
    Critical = 0x1 << 4,
    All = 0xF
};

export constexpr auto kInfo = LogLevelBits::Info;
export constexpr auto kWarning = LogLevelBits::Warning;
export constexpr auto kError = LogLevelBits::Error;
export constexpr auto kCritical = LogLevelBits::Critical;

DECLARE_FLAG_TYPE_PRIVATE(LogLevel, LogLevelBits)

namespace Log {
export class CRVM_API Logger {
public:
    Logger() = delete;
    explicit Logger(const eastl::string& Name);

    ~Logger();

    auto operator->() const -> spdlog::logger*;

private:
    class Impl;
    eastl::unique_ptr<Impl> pImpl;
};
}
}
