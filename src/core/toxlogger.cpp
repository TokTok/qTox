/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "toxlogger.h"

#include <QDebug>
#include <QLoggingCategory>

#include <atomic>
#include <cstdint>
#include <tox/tox.h>

namespace ToxLogger {
namespace {

Q_LOGGING_CATEGORY(toxcore, "tox.core")

// trace level generates too much noise to enable by default
std::atomic<Tox_Log_Level> minLevel{TOX_LOG_LEVEL_DEBUG};

/**
 * @brief Log message handler for toxcore log messages
 * @note See tox.h for the parameter definitions
 */
void onLogMessage(Tox* tox, Tox_Log_Level level, const char* file, uint32_t line, const char* func,
                  const char* message, void* user_data)
{
    std::ignore = tox;
    std::ignore = user_data;

    if (level < minLevel) {
        return;
    }

    switch (level) {
    case TOX_LOG_LEVEL_TRACE:
    case TOX_LOG_LEVEL_DEBUG:
        QMessageLogger(file, line, func).debug(toxcore) << message;
        break;
    case TOX_LOG_LEVEL_INFO:
        QMessageLogger(file, line, func).info(toxcore) << message;
        break;
    case TOX_LOG_LEVEL_WARNING:
        QMessageLogger(file, line, func).warning(toxcore) << message;
        break;
    case TOX_LOG_LEVEL_ERROR:
        QMessageLogger(file, line, func).critical(toxcore) << message;
        break;
    }
}

} // namespace

/**
 * @brief Sets the least severe toxcore messages to log.
 */
void setMinLevel(IDebugSettings::ToxcoreLogLevel level)
{
    switch (level) {
    case IDebugSettings::ToxcoreLogLevel::Trace:
        minLevel = TOX_LOG_LEVEL_TRACE;
        return;
    case IDebugSettings::ToxcoreLogLevel::Debug:
        minLevel = TOX_LOG_LEVEL_DEBUG;
        return;
    case IDebugSettings::ToxcoreLogLevel::Info:
        minLevel = TOX_LOG_LEVEL_INFO;
        return;
    case IDebugSettings::ToxcoreLogLevel::Warning:
        minLevel = TOX_LOG_LEVEL_WARNING;
        return;
    case IDebugSettings::ToxcoreLogLevel::Error:
        minLevel = TOX_LOG_LEVEL_ERROR;
        return;
    }
}

/**
 * @brief Makes toxcore log through qTox's message handler.
 */
void setLogCallback(Tox_Options* options)
{
    tox_options_set_log_callback(options, onLogMessage);
}

} // namespace ToxLogger
