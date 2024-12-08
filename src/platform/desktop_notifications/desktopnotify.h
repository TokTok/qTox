/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/notificationdata.h"

#include <QObject>

#include <memory>

class QSystemTrayIcon;
class Settings;

/**
 * @brief Show desktop notifications.
 *
 * Icon is optional, if not provided, no notifications will be shown unless
 * another backend exists.
 *
 * If DBus is available and a desktop notification server responds to us,
 * it is preferred over tray notifications.
 */
class DesktopNotify : public QObject
{
    Q_OBJECT

public:
    DesktopNotify(Settings& settings, QObject* parent);
    ~DesktopNotify();

    void setIcon(QSystemTrayIcon* icon);

public slots:
    void notifyMessage(const NotificationData& notificationData);

signals:
    void notificationClosed();

private:
    struct Private;
    const std::unique_ptr<Private> d;
};
