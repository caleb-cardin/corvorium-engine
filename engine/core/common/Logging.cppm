// Copyright © 2026 Caleb Cardin. All rights reserved.
module;

#include "common/FlagTypeMacro.hpp"


#include <spdlog/pattern_formatter.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <string>
#include <memory>
#include <source_location>

export module Common:Logging;
import :Types;
import :Build;
import :Debug;
import :Flags;

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
export class Logger {
public:
    Logger() = delete;
    explicit Logger(std::string&& Name);

    ~Logger() = default;

    auto operator->() const -> spdlog::logger*
    {
        return pLogger.get();
    }

private:
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> pSink;
    std::unique_ptr<spdlog::logger> pLogger;
};

std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> g_loggersColorSink;

static auto GetGlobalLoggerSink() -> std::shared_ptr<spdlog::sinks::stdout_color_sink_mt>
{
    if (!g_loggersColorSink)
    {
        g_loggersColorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    }
    return g_loggersColorSink;
}

Logger::Logger(std::string&& Name)
    : pSink{GetGlobalLoggerSink()}
    , pLogger{std::make_unique<spdlog::logger>(std::forward<std::string>(Name), pSink)}
{
    pLogger->set_pattern("[%H:%M:%S] [%t] | [%n] %^[- %l -]%$ : %v");
    if constexpr (Build::DebugBuild())
    {
        pLogger->flush_on(spdlog::level::n_levels);
    }
    if constexpr (Build::DevelopmentBuild())
    {
        pLogger->flush_on(spdlog::level::warn);
    }
    if constexpr (Build::ReleaseBuild())
    {
        pLogger->flush_on(spdlog::level::critical);
    }
}
}
}
