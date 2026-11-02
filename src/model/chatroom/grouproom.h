/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QObject>

class Chat;
class Core;
class IDialogsManager;
class Group;
class ToxPk;
class FriendList;

class GroupRoom final : public QObject
{
    Q_OBJECT
public:
    GroupRoom(Group* group_, IDialogsManager* dialogsManager_, Core& core_, FriendList& friendList);

    Chat* getChat();

    Group* getGroup();

    bool hasNewMessage() const;
    void resetEventFlags();

    bool friendExists(const ToxPk& pk);
    void inviteFriend(const ToxPk& pk);

    bool possibleToOpenInNewWindow() const;
    bool canBeRemovedFromWindow() const;
    void removeGroupFromDialogs();

private:
    Group* group{nullptr};
    IDialogsManager* dialogsManager{nullptr};
    Core& core;
    FriendList& friendList;
};
