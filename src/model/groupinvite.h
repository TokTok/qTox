/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

#include <cstdint>

class GroupInvite
{
public:
    GroupInvite() = default;
    GroupInvite(uint32_t friendId_, QByteArray inviteData_, QString groupName_);
    bool operator==(const GroupInvite& other) const;

    uint32_t getFriendId() const;
    QByteArray getInviteData() const;
    QString getGroupName() const;
    QDateTime getInviteDate() const;

private:
    uint32_t friendId{0};
    QByteArray inviteData;
    QString groupName;
    QDateTime date;
};
