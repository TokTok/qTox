/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/core/conferencetype.h"

#include <QByteArray>
#include <QDateTime>

#include <cstdint>

class ConferenceInvite
{
public:
    ConferenceInvite() = default;
    ConferenceInvite(uint32_t friendId_, ConferenceType inviteType, QByteArray data);
    bool operator==(const ConferenceInvite& other) const;

    uint32_t getFriendId() const;
    ConferenceType getType() const;
    QByteArray getInvite() const;
    QDateTime getInviteDate() const;

private:
    uint32_t friendId{0};
    ConferenceType type{ConferenceType::TEXT};
    QByteArray invite;
    QDateTime date;
};
