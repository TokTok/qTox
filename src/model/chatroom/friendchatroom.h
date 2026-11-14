/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QVector>

class Chat;
class Core;
class Friend;
class Group;
class GroupList;
class IDialogsManager;
class Conference;
class Settings;
class ConferenceList;

struct ConferenceToDisplay
{
    QString name;
    Conference* conference;
};

struct GroupToDisplay
{
    QString name;
    Group* group;
};

struct CircleToDisplay
{
    QString name;
    int circleId;
};

class FriendChatroom final : public QObject
{
    Q_OBJECT
public:
    FriendChatroom(Friend* frnd_, IDialogsManager* dialogsManager_, Core& core_,
                   Settings& settings_, ConferenceList& conferenceList, GroupList& groupList);

    Chat* getChat();

public slots:

    Friend* getFriend();

    void setActive(bool active);

    bool canBeInvited() const;

    int getCircleId() const;
    QString getCircleName() const;

    void inviteToNewConference();
    void inviteFriend(const Conference* conference);

    void inviteToNewGroup();
    void inviteFriend(const Group* group);

    bool autoAcceptEnabled() const;
    QString getAutoAcceptDir() const;
    void disableAutoAccept();
    void setAutoAcceptDir(const QString& dir);

    QVector<ConferenceToDisplay> getConferences() const;
    QVector<GroupToDisplay> getGroups() const;
    QVector<CircleToDisplay> getOtherCircles() const;

    void resetEventFlags();

    bool possibleToOpenInNewWindow() const;
    bool canBeRemovedFromWindow() const;
    bool friendCanBeRemoved() const;
    void removeFriendFromDialogs();

signals:
    void activeChanged(bool activated);

private:
    bool active{false};
    Friend* frnd{nullptr};
    IDialogsManager* dialogsManager{nullptr};
    Core& core;
    Settings& settings;
    ConferenceList& conferenceList;
    GroupList& groupList;
};
