/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "aboutfriend.h"

#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/ifriendsettings.h"
#include "src/persistence/profile.h"

AboutFriend::AboutFriend(const Friend* f_, IFriendSettings* const settings_, Profile& profile_)
    : f{f_}
    , settings{settings_}
    , profile{profile_}
{
    settings->connectTo_contactNoteChanged(this, [this](const ToxPk& pk, const QString& note) {
        std::ignore = pk;
        emit noteChanged(note);
    });
    settings->connectTo_autoAcceptCallChanged(this, [this](const ToxPk& pk,
                                                           IFriendSettings::AutoAcceptCallFlags flag) {
        std::ignore = pk;
        emit autoAcceptCallChanged(flag);
    });
    settings->connectTo_autoAcceptDirChanged(this, [this](const ToxPk& pk, const QString& dir) {
        std::ignore = pk;
        emit autoAcceptDirChanged(dir);
    });
    settings->connectTo_autoGroupInviteChanged(this, [this](const ToxPk& pk, bool enable) {
        std::ignore = pk;
        emit autoGroupInviteChanged(enable);
    });
}

QString AboutFriend::getName() const
{
    return f->getDisplayedName();
}

QString AboutFriend::getStatusMessage() const
{
    return f->getStatusMessage();
}

ToxPk AboutFriend::getPublicKey() const
{
    return f->getPublicKey();
}

QPixmap AboutFriend::getAvatar() const
{
    const ToxPk pk = f->getPublicKey();
    const QPixmap avatar = profile.loadAvatar(pk);
    return avatar.isNull() ? QPixmap(QStringLiteral(":/img/contact_dark.svg")) : avatar;
}

QString AboutFriend::getNote() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getContactNote(pk);
}

void AboutFriend::setNote(const QString& note)
{
    const ToxPk pk = f->getPublicKey();
    settings->setContactNote(pk, note);
    settings->saveFriendSettings(pk);
}

QString AboutFriend::getAutoAcceptDir() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getAutoAcceptDir(pk);
}

void AboutFriend::setAutoAcceptDir(const QString& path)
{
    const ToxPk pk = f->getPublicKey();
    settings->setAutoAcceptDir(pk, path);
    settings->saveFriendSettings(pk);
}

IFriendSettings::AutoAcceptCallFlags AboutFriend::getAutoAcceptCall() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getAutoAcceptCall(pk);
}

void AboutFriend::setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag)
{
    const ToxPk pk = f->getPublicKey();
    settings->setAutoAcceptCall(pk, flag);
    settings->saveFriendSettings(pk);
}

bool AboutFriend::getAutoGroupInvite() const
{
    const ToxPk pk = f->getPublicKey();
    return settings->getAutoGroupInvite(pk);
}

void AboutFriend::setAutoGroupInvite(bool enabled)
{
    const ToxPk pk = f->getPublicKey();
    settings->setAutoGroupInvite(pk, enabled);
    settings->saveFriendSettings(pk);
}

bool AboutFriend::clearHistory()
{
    const ToxPk pk = f->getPublicKey();
    History* const history = profile.getHistory();
    if (history) {
        history->removeChatHistory(pk);
        return true;
    }

    return false;
}

bool AboutFriend::isHistoryExistence()
{
    History* const history = profile.getHistory();
    if (history) {
        const ToxPk pk = f->getPublicKey();
        return history->historyExists(pk);
    }

    return false;
}
