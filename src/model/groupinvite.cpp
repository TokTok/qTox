/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "groupinvite.h"

#include <utility>

/**
 * @class GroupInvite
 *
 * @brief This class contains information needed to accept a group invite
 */

GroupInvite::GroupInvite(uint32_t friendId_, QByteArray inviteData_, QString groupName_)
    : friendId{friendId_}
    , inviteData{std::move(inviteData_)}
    , groupName{std::move(groupName_)}
    , date{QDateTime::currentDateTime()}
{
}

bool GroupInvite::operator==(const GroupInvite& other) const
{
    return friendId == other.friendId && inviteData == other.inviteData
           && groupName == other.groupName && date == other.date;
}

uint32_t GroupInvite::getFriendId() const
{
    return friendId;
}

QByteArray GroupInvite::getInviteData() const
{
    return inviteData;
}

QString GroupInvite::getGroupName() const
{
    return groupName;
}

QDateTime GroupInvite::getInviteDate() const
{
    return date;
}
