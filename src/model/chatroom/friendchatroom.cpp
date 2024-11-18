/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/model/chatroom/friendchatroom.h"
#include "src/conferencelist.h"
#include "src/core/core.h"
#include "src/model/conference.h"
#include "src/model/dialogs/idialogsmanager.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/contentdialog.h"

#include <QCollator>

namespace {

QString getShortName(const QString& name)
{
    constexpr auto MAX_NAME_LENGTH = 30;
    if (name.length() <= MAX_NAME_LENGTH) {
        return name;
    }

    return name.left(MAX_NAME_LENGTH).trimmed() + "…";
}

} // namespace

FriendChatroom::FriendChatroom(Friend* frnd_, IDialogsManager* dialogsManager_, Core& core_,
                               Settings& settings_, ConferenceList& conferenceList_)
    : frnd{frnd_}
    , dialogsManager{dialogsManager_}
    , core{core_}
    , settings{settings_}
    , conferenceList{conferenceList_}
{
}

Friend* FriendChatroom::getFriend()
{
    return frnd;
}

Chat* FriendChatroom::getChat()
{
    return frnd;
}

void FriendChatroom::setActive(bool _active)
{
    if (active != _active) {
        active = _active;
        emit activeChanged(active);
    }
}

bool FriendChatroom::canBeInvited() const
{
    return Status::isOnline(frnd->getStatus());
}

int FriendChatroom::getCircleId() const
{
    return settings.getFriendCircleID(frnd->getPublicKey());
}

QString FriendChatroom::getCircleName() const
{
    const auto circleId = getCircleId();
    return settings.getCircleName(circleId);
}

void FriendChatroom::inviteToNewConference()
{
    const auto friendId = frnd->getId();
    const auto conferenceId = core.createConference();
    core.conferenceInviteFriend(friendId, conferenceId);
}

QString FriendChatroom::getAutoAcceptDir() const
{
    const auto pk = frnd->getPublicKey();
    return settings.getAutoAcceptDir(pk);
}

void FriendChatroom::setAutoAcceptDir(const QString& dir)
{
    const auto pk = frnd->getPublicKey();
    settings.setAutoAcceptDir(pk, dir);
}

void FriendChatroom::disableAutoAccept()
{
    setAutoAcceptDir(QString{});
}

bool FriendChatroom::autoAcceptEnabled() const
{
    return getAutoAcceptDir().isEmpty();
}

void FriendChatroom::inviteFriend(const Conference* conference)
{
    const auto friendId = frnd->getId();
    const auto conferenceId = conference->getId();
    core.conferenceInviteFriend(friendId, conferenceId);
}

QVector<ConferenceToDisplay> FriendChatroom::getConferences() const
{
    QVector<ConferenceToDisplay> conferences;
    for (const auto conference : conferenceList.getAllConferences()) {
        const auto name = getShortName(conference->getName());
        const ConferenceToDisplay conferenceToDisplay = {name, conference};
        conferences.push_back(conferenceToDisplay);
    }

    return conferences;
}

/**
 * @brief Return sorted list of circles exclude current circle.
 */
QVector<CircleToDisplay> FriendChatroom::getOtherCircles() const
{
    QVector<CircleToDisplay> circles;
    const auto currentCircleId = getCircleId();
    for (int i = 0; i < settings.getCircleCount(); ++i) {
        if (i == currentCircleId) {
            continue;
        }

        const auto name = getShortName(settings.getCircleName(i));
        const CircleToDisplay circle = {name, i};
        circles.push_back(circle);
    }

    std::sort(circles.begin(), circles.end(),
              [](const CircleToDisplay& a, const CircleToDisplay& b) -> bool {
                  QCollator collator;
                  collator.setNumericMode(true);
                  return collator.compare(a.name, b.name) < 0;
              });

    return circles;
}

void FriendChatroom::resetEventFlags()
{
    frnd->setEventFlag(false);
}

bool FriendChatroom::possibleToOpenInNewWindow() const
{
    const auto friendPk = frnd->getPublicKey();
    const auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    return (dialogs == nullptr) || dialogs->chatroomCount() > 1;
}

bool FriendChatroom::canBeRemovedFromWindow() const
{
    const auto friendPk = frnd->getPublicKey();
    const auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    return (dialogs != nullptr) && dialogs->hasChat(friendPk);
}

bool FriendChatroom::friendCanBeRemoved() const
{
    const auto friendPk = frnd->getPublicKey();
    const auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    return (dialogs == nullptr) || !dialogs->hasChat(friendPk);
}

void FriendChatroom::removeFriendFromDialogs()
{
    const auto friendPk = frnd->getPublicKey();
    auto dialogs = dialogsManager->getFriendDialogs(friendPk);
    dialogs->removeFriend(friendPk);
}
