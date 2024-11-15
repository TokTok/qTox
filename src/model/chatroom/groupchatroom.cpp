/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "groupchatroom.h"

#include "src/core/core.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"

GroupChatroom::GroupChatroom(Group* group_, IDialogsManager* dialogsManager_, Core& core_,
                             FriendList& friendList_)
    : group{group_}
    , dialogsManager{dialogsManager_}
    , core{core_}
    , friendList{friendList_}
{
}

Chat* GroupChatroom::getChat()
{
    return group;
}

Group* GroupChatroom::getGroup()
{
    return group;
}

bool GroupChatroom::hasNewMessage() const
{
    return group->getEventFlag();
}

void GroupChatroom::resetEventFlags()
{
    group->setEventFlag(false);
    group->setMentionedFlag(false);
}

bool GroupChatroom::friendExists(const ToxPk& pk)
{
    return friendList.findFriend(pk) != nullptr;
}

void GroupChatroom::inviteFriend(const ToxPk& pk)
{
    const Friend* frnd = friendList.findFriend(pk);
    const auto friendId = frnd->getId();
    const auto groupId = group->getId();
    const auto canInvite = Status::isOnline(frnd->getStatus());

    if (canInvite) {
        core.groupInviteFriend(friendId, groupId);
    }
}

bool GroupChatroom::possibleToOpenInNewWindow() const
{
    const auto groupId = group->getPersistentId();
    const auto dialogs = dialogsManager->getGroupDialogs(groupId);
    return !dialogs || dialogs->chatroomCount() > 1;
}

bool GroupChatroom::canBeRemovedFromWindow() const
{
    const auto groupId = group->getPersistentId();
    const auto dialogs = dialogsManager->getGroupDialogs(groupId);
    return dialogs && dialogs->hasChat(groupId);
}

void GroupChatroom::removeGroupFromDialogs()
{
    const auto groupId = group->getPersistentId();
    auto dialogs = dialogsManager->getGroupDialogs(groupId);
    dialogs->removeGroup(groupId);
}
