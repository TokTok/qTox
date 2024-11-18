/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "notificationgenerator.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "util/display.h"

#include <QCollator>

namespace {
size_t getNumMessages(const QHash<const Friend*, size_t>& friendNotifications,
                      const QHash<const Conference*, size_t>& conferenceNotifications)
{
    auto numMessages = std::accumulate(friendNotifications.begin(), friendNotifications.end(), 0);
    numMessages = std::accumulate(conferenceNotifications.begin(), conferenceNotifications.end(), numMessages);

    return numMessages;
}

size_t getNumChats(const QHash<const Friend*, size_t>& friendNotifications,
                   const QHash<const Conference*, size_t>& conferenceNotifications)
{
    return friendNotifications.size() + conferenceNotifications.size();
}

QString generateMultiChatTitle(size_t numChats, size_t numMessages)
{
    //: e.g. 3 messages from 2 chats
    return QObject::tr("%1 message(s) from %2 chats").arg(numMessages).arg(numChats);
}

template <typename T>
QString generateSingleChatTitle(const QHash<T, size_t> numNotifications, T contact)
{
    if (numNotifications[contact] > 1) {
        //: e.g. 2 messages from Bob
        return QObject::tr("%1 message(s) from %2")
            .arg(numNotifications[contact])
            .arg(contact->getDisplayedName());
    } else {
        return contact->getDisplayedName();
    }
}

QString generateTitle(const QHash<const Friend*, size_t>& friendNotifications,
                      const QHash<const Conference*, size_t>& conferenceNotifications, const Friend* f)
{
    auto numChats = getNumChats(friendNotifications, conferenceNotifications);
    if (numChats > 1) {
        return generateMultiChatTitle(numChats,
                                      getNumMessages(friendNotifications, conferenceNotifications));
    } else {
        return generateSingleChatTitle(friendNotifications, f);
    }
}

QString generateTitle(const QHash<const Friend*, size_t>& friendNotifications,
                      const QHash<const Conference*, size_t>& conferenceNotifications, const Conference* c)
{
    auto numChats = getNumChats(friendNotifications, conferenceNotifications);
    if (numChats > 1) {
        return generateMultiChatTitle(numChats,
                                      getNumMessages(friendNotifications, conferenceNotifications));
    } else {
        return generateSingleChatTitle(conferenceNotifications, c);
    }
}

QString generateContent(const QHash<const Friend*, size_t>& friendNotifications,
                        const QHash<const Conference*, size_t>& conferenceNotifications, QString lastMessage,
                        const ToxPk& sender)
{
    assert(friendNotifications.size() > 0 || conferenceNotifications.size() > 0);

    auto numChats = getNumChats(friendNotifications, conferenceNotifications);
    if (numChats > 1) {
        // Copy all names into a vector to simplify formatting logic between
        // multiple lists
        std::vector<QString> displayNames;
        displayNames.reserve(numChats);

        for (auto it = friendNotifications.begin(); it != friendNotifications.end(); ++it) {
            displayNames.push_back(it.key()->getDisplayedName());
        }

        for (auto it = conferenceNotifications.begin(); it != conferenceNotifications.end(); ++it) {
            displayNames.push_back(it.key()->getDisplayedName());
        }

        assert(displayNames.size() > 0);

        // Lexiographically sort all display names to ensure consistent formatting
        QCollator collator;
        std::sort(displayNames.begin(), displayNames.end(),
                  [&](const QString& a, const QString& b) { return collator.compare(a, b) < 1; });

        auto it = displayNames.begin();

        QString ret = *it;

        while (++it != displayNames.end()) {
            ret += ", " + *it;
        }

        return ret;
    } else if (conferenceNotifications.size() == 1) {
        auto it = conferenceNotifications.begin();
        if (it == conferenceNotifications.end()) {
            qFatal("Concurrency error: conference notifications got cleared while reading");
        }
        return it.key()->getPeerList()[sender] + ": " + lastMessage;
    } else {
        return lastMessage;
    }
}

QPixmap getSenderAvatar(Profile* profile, const ToxPk& sender)
{
    return profile ? profile->loadAvatar(sender) : QPixmap();
}
} // namespace

NotificationGenerator::NotificationGenerator(INotificationSettings const& notificationSettings_,
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

    ret.title = generateTitle(friendNotifications, conferenceNotifications, f);
    ret.message = generateContent(friendNotifications, conferenceNotifications, message, f->getPublicKey());
    ret.pixmap = getSenderAvatar(profile, f->getPublicKey());

    return ret;
}

NotificationData NotificationGenerator::conferenceMessageNotification(const Conference* c, const ToxPk& sender,
                                                                 const QString& message)
{
    conferenceNotifications[c]++;

    NotificationData ret;

    if (notificationSettings.getNotifyHide()) {
        ret.title = tr("New conference message");
        return ret;
    }

    ret.title = generateTitle(friendNotifications, conferenceNotifications, c);
    ret.message = generateContent(friendNotifications, conferenceNotifications, message, sender);
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

    auto numChats = getNumChats(friendNotifications, conferenceNotifications);
    auto numMessages = getNumMessages(friendNotifications, conferenceNotifications);

    if (numChats > 1 || numMessages > 1) {
        ret.title = generateTitle(friendNotifications, conferenceNotifications, f);
        ret.message = generateContent(friendNotifications, conferenceNotifications,
                                      tr("Incoming file transfer"), f->getPublicKey());
    } else {
        //: e.g. Bob - file transfer
        ret.title = tr("%1 - file transfer").arg(f->getDisplayedName());
        ret.message = filename + " (" + getHumanReadableSize(fileSize) + ")";
    }

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

    return ret;
}

void NotificationGenerator::onNotificationActivated()
{
    friendNotifications = {};
    conferenceNotifications = {};
}
