/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "notificationgenerator.h"

#include "util/display.h"

#include <QCollator>

namespace {

const QRegularExpression TRIFA_WRAPPER(QStringLiteral("[[]TRIfA_SUFFIX[]](.+)[[]/TRIfA_SUFFIX[]]$"),
                                       QRegularExpression::DotMatchesEverythingOption);

QString generateContent(const QHash<const Conference*, size_t>& conferenceNotifications,
                        QString lastMessage, const ToxPk& sender)
{
    assert(!conferenceNotifications.empty());

    auto it = conferenceNotifications.begin();
    if (it == conferenceNotifications.end()) {
        qFatal("Concurrency error: conference notifications got cleared while reading");
    }
    return it.key()->getPeerList()[sender] + ": " + lastMessage;
}

QPixmap getSenderAvatar(Profile* profile, const ToxPk& sender)
{
    return profile != nullptr ? profile->loadAvatar(sender) : QPixmap();
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
    ret.category = "im.received";

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("New message");
        return ret;
    }

    ret.title = f->getDisplayedName();
    // Make sure we have removed the TRIfA suffix if any.

    ret.message = QString(message).remove(TRIFA_WRAPPER);
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::incomingCallNotification(const Friend* f)
{
    friendNotifications[f]++;

    NotificationData ret;
    ret.category = "call.incoming";

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Incoming call");
        return ret;
    }

    ret.title = f->getDisplayedName();
    ret.message = tr("Incoming call");
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::conferenceMessageNotification(const Conference* c,
                                                                      const ToxPk& sender,
                                                                      const QString& message)
{
    conferenceNotifications[c]++;

    NotificationData ret;
    ret.category = "transfer";

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("New conference message");
        return ret;
    }

    ret.title = c->getDisplayedName();
    ret.message = generateContent(conferenceNotifications, message, sender);
    ret.pixmap = getSenderAvatar(profile, sender);

    return ret;
}

NotificationData NotificationGenerator::fileTransferNotification(const Friend* f,
                                                                 const QString& filename,
                                                                 size_t fileSize)
{
    friendNotifications[f]++;

    NotificationData ret;
    ret.category = "transfer";

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Incoming file transfer");
        return ret;
    }

    //: e.g. Bob - file transfer
    ret.title = tr("%1 - file transfer").arg(f->getDisplayedName());
    ret.message = filename + " (" + getHumanReadableSize(fileSize) + ")";
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::conferenceInvitationNotification(const Friend* from)
{
    NotificationData ret;
    ret.category = "im";

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Conference invite received");
        return ret;
    }

    ret.title = tr("%1 invites you to join a conference.").arg(from->getDisplayedName());
    ret.message = "";
    ret.pixmap = getSenderAvatar(profile, from->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::friendRequestNotification(const ToxPk& sender,
                                                                  const QString& message)
{
    NotificationData ret;
    ret.category = "im";

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("Friend request received");
        return ret;
    }

    ret.title = tr("Friend request received from %1").arg(sender.toString());
    ret.message = message;

    return ret;
}

void NotificationGenerator::onNotificationActivated()
{
    friendNotifications = {};
    conferenceNotifications = {};
}
