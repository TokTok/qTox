/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "groupinvite.h"

#include <utility>

/**
 * @class GroupInvite
 *
 * @brief This class contains information needed to create a group invite
 */

GroupInvite::GroupInvite(uint32_t friendId_, uint8_t inviteType, QByteArray data)
    : friendId{friendId_}
    , type{inviteType}
    , invite{std::move(data)}
    , date{QDateTime::currentDateTime()}
{
}

bool GroupInvite::operator==(const GroupInvite& other) const
{
    return friendId == other.friendId && type == other.type && invite == other.invite
           && date == other.date;
}

uint32_t GroupInvite::getFriendId() const
{
    return friendId;
}

uint8_t GroupInvite::getType() const
{
    return type;
}

QByteArray GroupInvite::getInvite() const
{
    return invite;
}

QDateTime GroupInvite::getInviteDate() const
{
    return date;
}
