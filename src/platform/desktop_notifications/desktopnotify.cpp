/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#if DESKTOP_NOTIFICATIONS
#include "desktopnotify.h"

#include <src/persistence/settings.h>

#include <libsnore/snore.h>

#include <QDebug>
#include <QThread>

DesktopNotify::DesktopNotify(Settings& settings_)
    : notifyCore{Snore::SnoreCore::instance()}
    , snoreIcon{":/img/icons/qtox.svg"}
    , settings{settings_}
{

    notifyCore.loadPlugins(Snore::SnorePlugin::Backend);
    qDebug() << "primary notification backend:" << notifyCore.primaryNotificationBackend();

    snoreApp = Snore::Application("qTox", snoreIcon);

    notifyCore.registerApplication(snoreApp);

    connect(&notifyCore, &Snore::SnoreCore::notificationClosed, this, &DesktopNotify::onNotificationClose);
}

void DesktopNotify::notifyMessage(const NotificationData& notificationData)
{
    if(!(settings.getNotify() && settings.getDesktopNotify())) {
        return;
    }

    auto icon = notificationData.pixmap.isNull() ? snoreIcon : Snore::Icon(notificationData.pixmap);
    auto newNotification = Snore::Notification{snoreApp, Snore::Alert(), notificationData.title, notificationData.message, icon, 0};
    latestId = newNotification.id();

    if (lastNotification.isValid()) {
        // Workaround for broken updating behavior in snore. Snore increments
        // the message count when a notification is updated. Snore also caps the
        // number of outgoing messages at 3. This means that if we update
        // notifications more than 3 times we do not get notifications until the
        // user activates the notification.
        //
        // We work around this by closing the existing notification and replacing
        // it with a new one. We then only process the notification close if the
        // latest notification id is the same as the one we are closing. This allows
        // us to continue counting how many unread messages a user has until they
        // close the notification themselves.
        //
        // I've filed a bug on the snorenotify mailing list but the project seems
        // pretty dead. I filed a ticket on March 11 2020, and as of April 5 2020
        // the moderators have not even acknowledged the message. A previous message
        // got a response starting with "Snorenotify isn't that well maintained any more"
        // (see https://mail.kde.org/pipermail/snorenotify/2019-March/000004.html)
        // so I don't have hope of this being fixed any time soon
        notifyCore.requestCloseNotification(lastNotification, Snore::Notification::CloseReasons::Dismissed);
    }

    notifyCore.broadcastNotification(newNotification);
    lastNotification = newNotification;
}

void DesktopNotify::onNotificationClose(Snore::Notification notification)
{
    if (notification.id() == latestId) {
        lastNotification = {};
        emit notificationClosed();
    }
}
#endif
