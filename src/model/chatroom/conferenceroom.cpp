/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "conferenceroom.h"

#include "src/core/core.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/model/conference.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"

ConferenceRoom::ConferenceRoom(Conference* conference_, IDialogsManager* dialogsManager_,
                               Core& core_, FriendList& friendList_)
    : conference{conference_}
    , dialogsManager{dialogsManager_}
    , core{core_}
    , friendList{friendList_}
{
}

Chat* ConferenceRoom::getChat()
{
    return conference;
}

Conference* ConferenceRoom::getConference()
{
    return conference;
}

bool ConferenceRoom::hasNewMessage() const
{
    return conference->getEventFlag();
}

void ConferenceRoom::resetEventFlags()
{
    conference->setEventFlag(false);
    conference->setMentionedFlag(false);
}

bool ConferenceRoom::friendExists(const ToxPk& pk)
{
    return friendList.findFriend(pk) != nullptr;
}

void ConferenceRoom::inviteFriend(const ToxPk& pk)
{
    const Friend* frnd = friendList.findFriend(pk);
    const auto friendId = frnd->getId();
    const auto conferenceId = conference->getId();
    const auto canInvite = Status::isOnline(frnd->getStatus());

    if (canInvite) {
        core.conferenceInviteFriend(friendId, conferenceId);
    }
}

bool ConferenceRoom::possibleToOpenInNewWindow() const
{
    const auto conferenceId = conference->getPersistentId();
    const auto dialogs = dialogsManager->getConferenceDialogs(conferenceId);
    return (dialogs == nullptr) || dialogs->chatroomCount() > 1;
}

bool ConferenceRoom::canBeRemovedFromWindow() const
{
    const auto conferenceId = conference->getPersistentId();
    const auto dialogs = dialogsManager->getConferenceDialogs(conferenceId);
    return (dialogs != nullptr) && dialogs->hasChat(conferenceId);
}

void ConferenceRoom::removeConferenceFromDialogs()
{
    const auto conferenceId = conference->getPersistentId();
    auto dialogs = dialogsManager->getConferenceDialogs(conferenceId);
    dialogs->removeConference(conferenceId);
}
