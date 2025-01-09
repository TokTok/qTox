/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "toxlogger.h"

#include <QDebug>
#include <QLoggingCategory>

#include <tox/tox.h>

namespace ToxLogger {
namespace {

Q_LOGGING_CATEGORY(toxcore, "tox.core")

} // namespace

/**
 * @brief Log message handler for toxcore log messages
 * @note See tox.h for the parameter definitions
 */
void onLogMessage(Tox* tox, Tox_Log_Level level, const char* file, uint32_t line, const char* func,
                  const char* message, void* user_data)
{
    std::ignore = tox;
    std::ignore = user_data;

    switch (level) {
    case TOX_LOG_LEVEL_TRACE:
        return; // trace level generates too much noise to enable by default
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

} // namespace ToxLogger
