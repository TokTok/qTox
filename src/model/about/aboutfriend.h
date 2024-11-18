/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "iaboutfriend.h"
#include "src/persistence/ifriendsettings.h"
#include "util/interface.h"

#include <QObject>

class Friend;
class IFriendSettings;
class Profile;

class AboutFriend : public QObject, public IAboutFriend
{
    Q_OBJECT

public:
    AboutFriend(const Friend* f_, IFriendSettings* const settings, Profile& profile);

    QString getName() const override;
    QString getStatusMessage() const override;
    ToxPk getPublicKey() const override;

    QPixmap getAvatar() const override;

    QString getNote() const override;
    void setNote(const QString& note) override;

    QString getAutoAcceptDir() const override;
    void setAutoAcceptDir(const QString& path) override;

    IFriendSettings::AutoAcceptCallFlags getAutoAcceptCall() const override;
    void setAutoAcceptCall(IFriendSettings::AutoAcceptCallFlags flag) override;

    bool getAutoConferenceInvite() const override;
    void setAutoConferenceInvite(bool enabled) override;

    bool clearHistory() override;
    bool isHistoryExistence() override;

    SIGNAL_IMPL(AboutFriend, nameChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, statusChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, publicKeyChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, avatarChanged, const QPixmap&)
    SIGNAL_IMPL(AboutFriend, noteChanged, const QString&)

    SIGNAL_IMPL(AboutFriend, autoAcceptDirChanged, const QString&)
    SIGNAL_IMPL(AboutFriend, autoAcceptCallChanged, IFriendSettings::AutoAcceptCallFlags)
    SIGNAL_IMPL(AboutFriend, autoConferenceInviteChanged, bool)

private:
    const Friend* const f;
    IFriendSettings* const settings;
    Profile& profile;
};
