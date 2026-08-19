// Copyright © 2026 Caleb Cardin. All rights reserved.
module;
#include <spdlog/pattern_formatter.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <source_location>

#include "EASTL/string.h"
#include "EASTL/unique_ptr.h"
module Common:LoggingImpl;
import :Logging;

namespace Corvorium::Log {
class Logger::Impl {
public:
    Impl(eastl::string&& Name)
        : pLogger{eastl::make_unique<spdlog::logger>(std::forward<std::string>(Name), Impl::pSink)}
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

    spdlog::logger* Get() const
    {
        return pLogger.get();
    }

private:
    friend class Corvorium::Log::Logger;
    inline static std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> pSink;
    std::unique_ptr<spdlog::logger> pLogger;
};

spdlog::logger* Logger::operator->() const
{
    return pImpl->Get();
}

Logger::Logger(eastl::string&& Name)
    : pImpl{eastl::make_unique<Impl>(eastl::move(Name))}
{}

Logger::~Logger() = default;
}
