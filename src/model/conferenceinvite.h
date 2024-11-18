/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <cstdint>

class ConferenceInvite
{
public:
    ConferenceInvite() = default;
    ConferenceInvite(uint32_t friendId_, uint8_t inviteType, const QByteArray& data);
    bool operator==(const ConferenceInvite& other) const;

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
