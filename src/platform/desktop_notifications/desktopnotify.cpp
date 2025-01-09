/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2025 The TokTok team.
 */

#include "desktopnotify.h"

#include "desktopnotifybackend.h"

#include "src/persistence/inotificationsettings.h"

#include <QSystemTrayIcon>

struct DesktopNotify::Private
{
    INotificationSettings& settings;
    QSystemTrayIcon* icon;
    DesktopNotifyBackend* dbus;
};

DesktopNotify::DesktopNotify(INotificationSettings& settings, QObject* parent)
    : QObject(parent)
    , d{std::make_unique<Private>(Private{
          settings,
          nullptr,
          new DesktopNotifyBackend(this),
      })}
{
    connect(d->dbus, &DesktopNotifyBackend::messageClicked, this, &DesktopNotify::notificationClosed);
    if (d->icon) {
        connect(d->icon, &QSystemTrayIcon::messageClicked, this, &DesktopNotify::notificationClosed);
    }
}

DesktopNotify::~DesktopNotify() = default;

void DesktopNotify::setIcon(QSystemTrayIcon* icon)
{
    d->icon = icon;
}

void DesktopNotify::notifyMessage(const NotificationData& notificationData)
{
    if (!(d->settings.getNotify() && d->settings.getDesktopNotify())) {
        return;
    }

    // Try system-backends first.
    if (d->settings.getNotifySystemBackend()) {
        if (d->dbus->showMessage(notificationData.title, notificationData.message,
                                 notificationData.pixmap)) {
            return;
        }
    }

    // Fallback to QSystemTrayIcon.
    if (d->icon) {
        d->icon->showMessage(notificationData.title, notificationData.message, notificationData.pixmap);
    }
}
