/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "friendlist.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include "src/core/chatid.h"
#include "src/core/toxpk.h"
#include <QDebug>
#include <QHash>
#include <QMenu>

Friend* FriendList::addFriend(uint32_t friendId, const ToxPk& friendPk, Settings& settings)
{
    auto friendChecker = friendList.find(friendPk);
    if (friendChecker != friendList.end()) {
        qWarning() << "addFriend: friendPk already taken";
    }

    QString alias = settings.getFriendAlias(friendPk);
    Friend* newfriend = new Friend(friendId, friendPk, alias);
    friendList[friendPk] = newfriend;
    id2key[friendId] = friendPk;

    return newfriend;
}

Friend* FriendList::findFriend(const ToxPk& friendPk)
{
    auto f_it = friendList.find(friendPk);
    if (f_it != friendList.end()) {
        return *f_it;
    }

    return nullptr;
}

const ToxPk& FriendList::id2Key(uint32_t friendId)
{
    return id2key[friendId];
}

void FriendList::removeFriend(const ToxPk& friendPk, Settings& settings, bool fake)
{
    auto f_it = friendList.find(friendPk);
    if (f_it != friendList.end()) {
        if (!fake)
            settings.removeFriendSettings(f_it.value()->getPublicKey());
        friendList.erase(f_it);
    }
}

void FriendList::clear()
{
    for (auto friendptr : friendList)
        delete friendptr;
    friendList.clear();
}

QList<Friend*> FriendList::getAllFriends()
{
    return friendList.values();
}

QString FriendList::decideNickname(const ToxPk& friendPk, const QString& origName)
{
    Friend* f = FriendList::findFriend(friendPk);
    if (f != nullptr) {
        return f->getDisplayedName();
    } else if (!origName.isEmpty()) {
        return origName;
    } else {
        return friendPk.toString();
    }
}
