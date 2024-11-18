/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "util/interface.h"

#include <QFlag>
#include <QObject>

class ToxPk;

class IFriendSettings
{
public:
    enum class AutoAcceptCall
    {
        None = 0x00,
        Audio = 0x01,
        Video = 0x02,
        AV = Audio | Video
    };
    Q_DECLARE_FLAGS(AutoAcceptCallFlags, AutoAcceptCall)

    IFriendSettings() = default;
    virtual ~IFriendSettings();
    IFriendSettings(const IFriendSettings&) = default;
    IFriendSettings& operator=(const IFriendSettings&) = default;
    IFriendSettings(IFriendSettings&&) = default;
    IFriendSettings& operator=(IFriendSettings&&) = default;

    virtual QString getContactNote(const ToxPk& pk) const = 0;
    virtual void setContactNote(const ToxPk& pk, const QString& note) = 0;

    virtual QString getAutoAcceptDir(const ToxPk& pk) const = 0;
    virtual void setAutoAcceptDir(const ToxPk& pk, const QString& dir) = 0;

    virtual AutoAcceptCallFlags getAutoAcceptCall(const ToxPk& pk) const = 0;
    virtual void setAutoAcceptCall(const ToxPk& pk, AutoAcceptCallFlags accept) = 0;

    virtual bool getAutoConferenceInvite(const ToxPk& pk) const = 0;
    virtual void setAutoConferenceInvite(const ToxPk& pk, bool accept) = 0;

    virtual QString getFriendAlias(const ToxPk& pk) const = 0;
    virtual void setFriendAlias(const ToxPk& pk, const QString& alias) = 0;

    virtual int getFriendCircleID(const ToxPk& pk) const = 0;
    virtual void setFriendCircleID(const ToxPk& pk, int circleID) = 0;

    virtual QDateTime getFriendActivity(const ToxPk& pk) const = 0;
    virtual void setFriendActivity(const ToxPk& pk, const QDateTime& date) = 0;

    virtual void saveFriendSettings(const ToxPk& pk) = 0;
    virtual void removeFriendSettings(const ToxPk& pk) = 0;

signals:
    DECLARE_SIGNAL(autoAcceptCallChanged, const ToxPk& pk, AutoAcceptCallFlags accept);
    DECLARE_SIGNAL(autoConferenceInviteChanged, const ToxPk& pk, bool accept);
    DECLARE_SIGNAL(autoAcceptDirChanged, const ToxPk& pk, const QString& dir);
    DECLARE_SIGNAL(contactNoteChanged, const ToxPk& pk, const QString& note);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IFriendSettings::AutoAcceptCallFlags)
