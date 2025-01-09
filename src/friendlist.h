/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QHash>

#include <cstdint>

template <class T>
class QList;
class Friend;
class QByteArray;
class QString;
class ToxPk;
class Settings;

class FriendList
{
public:
    Friend* addFriend(uint32_t friendId, const ToxPk& friendPk, Settings& settings);
    Friend* findFriend(const ToxPk& friendPk);
    const ToxPk& id2Key(uint32_t friendId);
    QList<Friend*> getAllFriends();
    void removeFriend(const ToxPk& friendPk, Settings& settings, bool fake = false);
    void clear();
    QString decideNickname(const ToxPk& friendPk, const QString& origName);

private:
    QHash<ToxPk, Friend*> friendList;
    QHash<uint32_t, ToxPk> id2key;
};
