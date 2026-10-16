/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "desktopnotify.h"

#include "desktopnotifybackend.h"

#include "src/chatlog/textformatter.h"
#include "src/persistence/inotificationsettings.h"

#include <QDebug>
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
    if (d->icon != nullptr) {
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

    const QString message = d->settings.getHidePostNullSuffix()
                                ? TextFormatter::processPostNullSuffix(notificationData.message, false)
                                : notificationData.message;

    // Try system-backends first.
    if (d->settings.getNotifySystemBackend()) {
        if (d->dbus->showMessage(notificationData.title, message, notificationData.category,
                                 notificationData.pixmap)) {
            return;
        }
    }

    if (!QSystemTrayIcon::supportsMessages()) {
        qWarning() << "System does not support notifications";
        return;
    }

    if (d->icon == nullptr) {
        qWarning() << "System tray not yet initialised";
        return;
    }

    // Fallback to QSystemTrayIcon.
    d->icon->showMessage(notificationData.title,
                         d->dbus->supportsBodyMarkup() ? message.toHtmlEscaped() : message,
                         notificationData.pixmap);
}
