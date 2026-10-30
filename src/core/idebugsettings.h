/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "util/interface.h"

class IDebugSettings
{
public:
    // Stored in qtox.ini and used as combo box indices.
    enum class ToxcoreLogLevel
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4
    };
    IDebugSettings() = default;
    virtual ~IDebugSettings();
    IDebugSettings(const IDebugSettings&) = default;
    IDebugSettings& operator=(const IDebugSettings&) = default;
    IDebugSettings(IDebugSettings&&) = default;
    IDebugSettings& operator=(IDebugSettings&&) = default;

    virtual bool getEnableDebug() const = 0;
    virtual void setEnableDebug(bool enable) = 0;

    DECLARE_SIGNAL(enableDebugChanged, bool enabled);

    virtual ToxcoreLogLevel getToxcoreLogLevel() const = 0;
    virtual void setToxcoreLogLevel(ToxcoreLogLevel level) = 0;

    DECLARE_SIGNAL(toxcoreLogLevelChanged, IDebugSettings::ToxcoreLogLevel level);

    virtual bool getLogToFile() const = 0;
    virtual void setLogToFile(bool enabled) = 0;

    DECLARE_SIGNAL(logToFileChanged, bool enabled);
};
