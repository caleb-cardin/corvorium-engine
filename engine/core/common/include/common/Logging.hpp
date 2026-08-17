// Copyright © 2026 Caleb Cardin. All rights reserved.

#pragma once

// Formatted log
#define CRVM_LOG(_logger, _level, _msg, ...)                                                        \
do {                                                                                                \
    Detail::SourceLocation _loc = Detail::SourceLocation::current();                      \
    if constexpr (_level & LogLevelBits::Info)                                                      \
        _logger->info(_msg " @ {}:{}" __VA_OPT__(,) __VA_ARGS__, _loc.file_name(), _loc.line());    \
    if constexpr (_level & LogLevelBits::Warning)                                                   \
        _logger->warn(_msg " @ {}:{}" __VA_OPT__(,) __VA_ARGS__, _loc.file_name(), _loc.line());    \
    if constexpr (_level & LogLevelBits::Error)                                                     \
        _logger->error(_msg " @ {}:{}" __VA_OPT__(,) __VA_ARGS__, _loc.file_name(), _loc.line());   \
    if constexpr (_level & LogLevelBits::Critical)                                                  \
        _logger->critical(_msg " @ {}:{}" __VA_OPT__(,) __VA_ARGS__, _loc.file_name(), _loc.line());\
} while (0)


#define CRVM_LOG_RETURN_IF(_cond, _ret, _logger, _level, _msg, ...)             \
    do{if (_cond){                                                              \
            CRVM_LOG(_logger, _level, _msg __VA_OPT__(,) __VA_ARGS__);          \
            return _ret;                                                        \
        }} while (0)

#define CRVM_LOG_CONTINUE_IF(_cond, _logger, _level, _msg, ...)                      \
    do{if (_cond){                                                              \
            CRVM_LOG(_logger, _level, _msg __VA_OPT__(,) __VA_ARGS__);          \
            continue;                                                           \
        }} while (0)

#define CRVM_LOG_BREAK_IF(_cond, _logger, _level, _msg, ...)                         \
    do{if (_cond){                                                              \
            CRVM_LOG(_logger, _level, _msg __VA_OPT__(,) __VA_ARGS__);          \
            break;                                                              \
        }} while (0)


#if CORVORIUM_BUILD_MODE > 0
#define CRVM_LOG_DEBUG(_logger, _level, _msg, ...) CRVM_LOG(_logger, _level, _msg __VA_OPT__(,) __VA_ARGS__)

#else
#define CRVM_LOG_DEBUG(_logger, _level, _msg, ...)

#endif
