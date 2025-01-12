/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "notificationgenerator.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "util/display.h"

#include <QCollator>

namespace {

QString generateContent(const QHash<const Friend*, size_t>& friendNotifications,
                        const QHash<const Conference*, size_t>& conferenceNotifications,
                        QString lastMessage, const ToxPk& sender)
{
    assert(!friendNotifications.empty() || !conferenceNotifications.empty());

    if (conferenceNotifications.size() == 1) {
        auto it = conferenceNotifications.begin();
        if (it == conferenceNotifications.end()) {
            qFatal("Concurrency error: conference notifications got cleared while reading");
        }
        return it.key()->getPeerList()[sender] + ": " + lastMessage;
    }
    return lastMessage;
}

QPixmap getSenderAvatar(Profile* profile, const ToxPk& sender)
{
    return profile ? profile->loadAvatar(sender) : QPixmap();
}
} // namespace

NotificationGenerator::NotificationGenerator(const INotificationSettings& notificationSettings_,
                                             Profile* profile_)
    : notificationSettings(notificationSettings_)
    , profile(profile_)
{
}

NotificationGenerator::~NotificationGenerator() = default;

NotificationData NotificationGenerator::friendMessageNotification(const Friend* f, const QString& message)
{
    friendNotifications[f]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("New message");
        return ret;
    }

    ret.title = f->getDisplayedName();
    ret.message = message;
    ret.category = "im.received";
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::incomingCallNotification(const Friend* f)
{
    friendNotifications[f]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Incoming call");
        ret.category = "call.incoming";
        return ret;
    }

    ret.title = f->getDisplayedName();
    ret.message = tr("Incoming call");
    ret.category = "call.incoming";
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::conferenceMessageNotification(const Conference* c,
                                                                      const ToxPk& sender,
                                                                      const QString& message)
{
    conferenceNotifications[c]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("New conference message");
        return ret;
    }

    ret.title = c->getDisplayedName();
    ret.message = generateContent(friendNotifications, conferenceNotifications, message, sender);
    ret.category = "im.received";
    ret.pixmap = getSenderAvatar(profile, sender);

    return ret;
}

NotificationData NotificationGenerator::fileTransferNotification(const Friend* f,
                                                                 const QString& filename,
                                                                 size_t fileSize)
{
    friendNotifications[f]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Incoming file transfer");
        return ret;
    }

    //: e.g. Bob - file transfer
    ret.title = tr("%1 - file transfer").arg(f->getDisplayedName());
    ret.message = filename + " (" + getHumanReadableSize(fileSize) + ")";
    ret.category = "transfer";
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::conferenceInvitationNotification(const Friend* from)
{
    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Conference invite received");
        return ret;
    }

    ret.title = tr("%1 invites you to join a conference.").arg(from->getDisplayedName());
    ret.message = "";
    ret.category = "im";
    ret.pixmap = getSenderAvatar(profile, from->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::friendRequestNotification(const ToxPk& sender,
                                                                  const QString& message)
{
    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Friend request received");
        return ret;
    }

    ret.title = tr("Friend request received from %1").arg(sender.toString());
    ret.message = message;
    ret.category = "im";

    return ret;
}

void NotificationGenerator::onNotificationActivated()
{
    friendNotifications = {};
    conferenceNotifications = {};
}
