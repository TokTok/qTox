/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#include "desktopnotify.h"

#include "desktopnotify_dbus.h"
#include "src/persistence/settings.h"

#include <QSystemTrayIcon>

struct DesktopNotify::Private
{
    Settings& settings;
    QSystemTrayIcon* icon;
    DesktopNotifyDBus* dbus;
};

DesktopNotify::DesktopNotify(Settings& settings, QObject* parent)
    : QObject(parent)
    , d{std::make_unique<Private>(Private{
          settings,
          nullptr,
          new DesktopNotifyDBus(this),
      })}
{
    connect(d->dbus, &DesktopNotifyDBus::messageClicked, this, &DesktopNotify::notificationClosed);
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

    // Try DBus first.
    if (d->dbus->showMessage(notificationData.title, notificationData.message, notificationData.pixmap)) {
        return;
    }

    // Fallback to QSystemTrayIcon.
    if (d->icon) {
        d->icon->showMessage(notificationData.title, notificationData.message, notificationData.pixmap);
    }
}
