/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "util/interface.h"

class IDebugSettings
{
public:
    IDebugSettings() = default;
    virtual ~IDebugSettings();
    IDebugSettings(const IDebugSettings&) = default;
    IDebugSettings& operator=(const IDebugSettings&) = default;
    IDebugSettings(IDebugSettings&&) = default;
    IDebugSettings& operator=(IDebugSettings&&) = default;

    virtual bool getEnableDebug() const = 0;
    virtual void setEnableDebug(bool enable) = 0;

    DECLARE_SIGNAL(enableDebugChanged, bool enabled);
};
