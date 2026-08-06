/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "grouproom.h"

#include "src/core/core.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"

GroupRoom::GroupRoom(Group* group_, IDialogsManager* dialogsManager_, Core& core_,
                     FriendList& friendList_)
    : group{group_}
    , dialogsManager{dialogsManager_}
    , core{core_}
    , friendList{friendList_}
{
}

Chat* GroupRoom::getChat()
{
    return group;
}

Group* GroupRoom::getGroup()
{
    return group;
}

bool GroupRoom::hasNewMessage() const
{
    return group->getEventFlag();
}

void GroupRoom::resetEventFlags()
{
    group->setEventFlag(false);
    group->setMentionedFlag(false);
}

bool GroupRoom::friendExists(const ToxPk& pk)
{
    return friendList.findFriend(pk) != nullptr;
}

void GroupRoom::inviteFriend(const ToxPk& pk)
{
    const Friend* frnd = friendList.findFriend(pk);
    const auto friendId = frnd->getId();
    const auto groupNumber = group->getId();
    const auto canInvite = Status::isOnline(frnd->getStatus());

    if (canInvite) {
        core.groupInviteFriend(friendId, groupNumber);
    }
}

bool GroupRoom::possibleToOpenInNewWindow() const
{
    const auto groupId = group->getPersistentId();
    auto* const dialogs = dialogsManager->getGroupDialogs(groupId);
    return (dialogs == nullptr) || dialogs->chatroomCount() > 1;
}

bool GroupRoom::canBeRemovedFromWindow() const
{
    const auto groupId = group->getPersistentId();
    auto* const dialogs = dialogsManager->getGroupDialogs(groupId);
    return (dialogs != nullptr) && dialogs->hasChat(groupId);
}

void GroupRoom::removeGroupFromDialogs()
{
    const auto groupId = group->getPersistentId();
    auto* dialogs = dialogsManager->getGroupDialogs(groupId);
    dialogs->removeGroup(groupId);
}
