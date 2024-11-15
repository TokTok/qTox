/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "grouplist.h"
#include "src/core/core.h"
#include "src/model/group.h"
#include <QDebug>
#include <QHash>

Group* GroupList::addGroup(Core& core, int groupNum, const GroupId& groupId, const QString& name,
                           bool isAvGroupchat, const QString& selfName, FriendList& friendList)
{
    auto checker = groupList.find(groupId);
    if (checker != groupList.end()) {
        qWarning() << "addGroup: groupId already taken";
    }

    Group* newGroup =
        new Group(groupNum, groupId, name, isAvGroupchat, selfName, core, core, friendList);
    groupList[groupId] = newGroup;
    id2key[groupNum] = groupId;
    return newGroup;
}

Group* GroupList::findGroup(const GroupId& groupId)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end())
        return *g_it;

    return nullptr;
}

const GroupId& GroupList::id2Key(uint32_t groupNum)
{
    return id2key[groupNum];
}

void GroupList::removeGroup(const GroupId& groupId, bool /*fake*/)
{
    auto g_it = groupList.find(groupId);
    if (g_it != groupList.end()) {
        groupList.erase(g_it);
    }
}

QList<Group*> GroupList::getAllGroups()
{
    QList<Group*> res;

    for (auto it : groupList)
        res.append(it);

    return res;
}

void GroupList::clear()
{
    for (auto groupptr : groupList)
        delete groupptr;
    groupList.clear();
}
