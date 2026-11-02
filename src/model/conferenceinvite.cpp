/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "conferenceinvite.h"

#include <utility>

/**
 * @class ConferenceInvite
 *
 * @brief This class contains information needed to create a conference invite
 */

ConferenceInvite::ConferenceInvite(uint32_t friendId_, ConferenceType inviteType, QByteArray data)
    : friendId{friendId_}
    , type{inviteType}
    , invite{std::move(data)}
    , date{QDateTime::currentDateTime()}
{
}

bool ConferenceInvite::operator==(const ConferenceInvite& other) const
{
    return friendId == other.friendId && type == other.type && invite == other.invite
           && date == other.date;
}

uint32_t ConferenceInvite::getFriendId() const
{
    return friendId;
}

ConferenceType ConferenceInvite::getType() const
{
    return type;
}

QByteArray ConferenceInvite::getInvite() const
{
    return invite;
}

QDateTime ConferenceInvite::getInviteDate() const
{
    return date;
}
