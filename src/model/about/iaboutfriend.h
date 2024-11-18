/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/persistence/ifriendsettings.h"
#include "util/interface.h"

#include <QObject>

class IAboutFriend
{
public:
    IAboutFriend() = default;
    virtual ~IAboutFriend();
    IAboutFriend(const IAboutFriend&) = default;
    IAboutFriend& operator=(const IAboutFriend&) = default;
    IAboutFriend(IAboutFriend&&) = default;
    IAboutFriend& operator=(IAboutFriend&&) = default;

    virtual QString getName() const = 0;
    virtual QString getStatusMessage() const = 0;
    virtual ToxPk getPublicKey() const = 0;

    virtual QPixmap getAvatar() const = 0;

    virtual QString getNote() const = 0;
    virtual void setNote(const QString& note) = 0;

    virtual QString getAutoAcceptDir() const = 0;
    virtual void setAutoAcceptDir(const QString& path) = 0;

    virtual IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const = 0;
    virtual void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) = 0;

    virtual bool getAutoConferenceInvite() const = 0;
    virtual void setAutoConferenceInvite(bool enabled) = 0;

    virtual bool clearHistory() = 0;
    virtual bool isHistoryExistence() = 0;

    /* signals */
    DECLARE_SIGNAL(nameChanged, const QString&);
    DECLARE_SIGNAL(statusChanged, const QString&);
    DECLARE_SIGNAL(publicKeyChanged, const QString&);

    DECLARE_SIGNAL(avatarChanged, const QPixmap&);
    DECLARE_SIGNAL(noteChanged, const QString&);

    DECLARE_SIGNAL(autoAcceptDirChanged, const QString&);
    DECLARE_SIGNAL(autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags);
    DECLARE_SIGNAL(autoConferenceInviteChanged, bool);
};
