/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <cstdint>

class GroupInvite
{
public:
    GroupInvite() = default;
    GroupInvite(uint32_t friendId_, uint8_t inviteType, QByteArray data);
    bool operator==(const GroupInvite& other) const;

    uint32_t getFriendId() const;
    uint8_t getType() const;
    QByteArray getInvite() const;
    QDateTime getInviteDate() const;

private:
    uint32_t friendId{0};
    uint8_t type{0};
    QByteArray invite;
    QDateTime date;
};
