/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "chatroom.h"

#include <QObject>
#include <QString>
#include <QVector>

class Core;
class IDialogsManager;
class Friend;
class Conference;
class Settings;
class ConferenceList;

struct ConferenceToDisplay
{
    QString name;
    Conference* conference;
};

struct CircleToDisplay
{
    QString name;
    int circleId;
};

class FriendChatroom : public QObject, public Chatroom
{
    Q_OBJECT
public:
    FriendChatroom(Friend* frnd_, IDialogsManager* dialogsManager_, Core& core_,
                   Settings& settings_, ConferenceList& conferenceList);

    Chat* getChat() override;

public slots:

    Friend* getFriend();

    void setActive(bool active);

    bool canBeInvited() const;

    int getCircleId() const;
    QString getCircleName() const;

    void inviteToNewConference();
    void inviteFriend(const Conference* conference);

    bool autoAcceptEnabled() const;
    QString getAutoAcceptDir() const;
    void disableAutoAccept();
    void setAutoAcceptDir(const QString& dir);

    QVector<ConferenceToDisplay> getConferences() const;
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
};
