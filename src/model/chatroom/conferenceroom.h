/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "chatroom.h"

#include <QObject>

class Core;
class IDialogsManager;
class Conference;
class ToxPk;
class FriendList;

class ConferenceRoom : public QObject, public Chatroom
{
    Q_OBJECT
public:
    ConferenceRoom(Conference* conference_, IDialogsManager* dialogsManager_, Core& core_, FriendList& friendList);

    Chat* getChat() override;

    Conference* getConference();

    bool hasNewMessage() const;
    void resetEventFlags();

    bool friendExists(const ToxPk& pk);
    void inviteFriend(const ToxPk& pk);

    bool possibleToOpenInNewWindow() const;
    bool canBeRemovedFromWindow() const;
    void removeConferenceFromDialogs();

private:
    Conference* conference{nullptr};
    IDialogsManager* dialogsManager{nullptr};
    Core& core;
    FriendList& friendList;
};
