/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/notificationdata.h"

#include <QObject>

class QSystemTrayIcon;
class Settings;

class DesktopNotify : public QObject
{
    Q_OBJECT
public:
    /**
     * Icon is optional, if not provided, no notifications will be shown.
     *
     * In the future, we may implement a fallback notification system if there's no
     * system tray available.
     */
    DesktopNotify(Settings& settings, QSystemTrayIcon* icon);
    ~DesktopNotify();

public slots:
    void notifyMessage(const NotificationData& notificationData);

signals:
    void notificationClosed();

private:
    Settings& settings_;
    QSystemTrayIcon* icon_;
};
