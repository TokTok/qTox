/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QStringList>

class INotificationSettings
{
public:
    INotificationSettings() = default;
    virtual ~INotificationSettings();
    INotificationSettings(const INotificationSettings&) = default;
    INotificationSettings& operator=(const INotificationSettings&) = default;
    INotificationSettings(INotificationSettings&&) = default;
    INotificationSettings& operator=(INotificationSettings&&) = default;

    virtual bool getNotify() const = 0;
    virtual void setNotify(bool newValue) = 0;

    virtual bool getShowWindow() const = 0;
    virtual void setShowWindow(bool newValue) = 0;

    virtual bool getDesktopNotify() const = 0;
    virtual void setDesktopNotify(bool enabled) = 0;

    virtual bool getNotifySound() const = 0;
    virtual void setNotifySound(bool newValue) = 0;

    virtual bool getNotifyHide() const = 0;
    virtual void setNotifyHide(bool newValue) = 0;

    virtual bool getBusySound() const = 0;
    virtual void setBusySound(bool newValue) = 0;

    virtual bool getConferenceAlwaysNotify() const = 0;
    virtual void setConferenceAlwaysNotify(bool newValue) = 0;
};
