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
export constexpr LogLevelBits Info = LogLevelBits::Info;
export constexpr LogLevelBits Warning = LogLevelBits::Warning;
export constexpr LogLevelBits Error = LogLevelBits::Error;
export constexpr LogLevelBits Critical = LogLevelBits::Critical;

DECLARE_FLAG_TYPE_PRIVATE(LogLevel, LogLevelBits)

namespace Log {

export class Logger {
public:
    Logger() = delete;
    explicit Logger(std::string&& Name);

    ~Logger() = default;

    auto operator->() const -> spdlog::logger*
    {
        return m_pLogger.get();
    }
private:
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_spSink;
    std::unique_ptr<spdlog::logger> m_pLogger;
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
    : m_spSink{GetGlobalLoggerSink()}
    , m_pLogger{std::make_unique<spdlog::logger>(std::forward<std::string>(Name), m_spSink)}
{
    m_pLogger->set_pattern("[%H:%M:%S] [%t] | [%n] %^[- %l -]%$ : %v");
    if constexpr (Build::DebugBuild())
    {
        m_pLogger->flush_on(spdlog::level::n_levels);
    }
    if constexpr (Build::DevelopmentBuild())
    {
        m_pLogger->flush_on(spdlog::level::warn);
    }
    if constexpr (Build::ReleaseBuild())
    {
        m_pLogger->flush_on(spdlog::level::critical);
    }
}
}
}
