/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 * Copyright © 2014-2019 by The qTox Project Contributors
 */

#pragma once

#include "src/core/conferenceid.h"
#include "src/core/receiptnum.h"
#include "src/core/toxpk.h"
#include "src/model/conferencemessagedispatcher.h"
#include "src/model/friendchatstate.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/message.h"

#include <QMap>
#include <QObject>

#include <map>
#include <memory>

class ChatHistory;
class Conference;
class ConferenceList;
class ConferenceRoom;
class Core;
class Friend;
class FriendChatroom;
class FriendList;
class IChatLog;
class IDialogsManager;
class Profile;
class Settings;

class ChatManager : public QObject
{
    Q_OBJECT

public:
    ChatManager(Profile& profile, Settings& settings, FriendList& friendList,
                ConferenceList& conferenceList, IDialogsManager* dialogsManager,
                QObject* parent = nullptr);

    void connectToCore(Core& core);

    FriendMessageDispatcher* getFriendDispatcher(const ToxPk& friendPk) const;
    ChatHistory* getFriendChatLog(const ToxPk& friendPk) const;
    std::shared_ptr<FriendChatroom> getFriendChatroom(const ToxPk& friendPk) const;
    FriendChatState* getFriendChatState(const ToxPk& friendPk) const;
    ConferenceMessageDispatcher* getConferenceDispatcher(const ConferenceId& id) const;
    IChatLog* getConferenceChatLog(const ConferenceId& id) const;
    std::shared_ptr<ConferenceRoom> getConferenceRoom(const ConferenceId& id) const;

    MessageProcessor::SharedParams& getSharedMessageProcessorParams();

    void removeFriend(const ToxPk& friendPk);
    void removeConference(const ConferenceId& conferenceId);
    void removeFriendModel(const ToxPk& friendPk);
    void removeConferenceModel(const ConferenceId& conferenceId);

signals:
    void friendAdded(Friend* f, std::shared_ptr<FriendChatroom> chatroom,
                     std::shared_ptr<FriendMessageDispatcher> dispatcher,
                     std::shared_ptr<ChatHistory> chatLog, FriendChatState* chatState);
    void friendRemoved(const ToxPk& friendPk);
    void conferenceAdded(Conference* c, std::shared_ptr<ConferenceRoom> chatroom,
                         std::shared_ptr<ConferenceMessageDispatcher> dispatcher,
                         std::shared_ptr<IChatLog> chatLog);
    void conferenceRemoved(const ConferenceId& conferenceId);
    void conferenceNeedsName(const ConferenceId& conferenceId);

private slots:
    void onFriendAdded(uint32_t friendId, const ToxPk& friendPk);
    void onFriendStatusChanged(uint32_t friendId, Status::Status status);
    void onFriendStatusMessageChanged(uint32_t friendId, const QString& message);
    void onFriendUsernameChanged(uint32_t friendId, const QString& username);

    void onFriendMessageReceived(uint32_t friendId, const QString& message, bool isAction);
    void onReceiptReceived(uint32_t friendId, ReceiptNum receipt);
    void onConferenceMessageReceived(uint32_t conferenceNum, uint32_t peerNum,
                                     const QString& message, bool isAction);

    void onEmptyConferenceCreated(uint32_t conferenceNum, const ConferenceId& id, const QString& title);
    void onConferenceJoined(uint32_t conferenceNum, const ConferenceId& id);
    void onConferencePeerlistChanged(uint32_t conferenceNum);
    void onConferencePeerNameChanged(uint32_t conferenceNum, const ToxPk& peerPk,
                                     const QString& newName);
    void onConferenceTitleChanged(uint32_t conferenceNum, const QString& author, const QString& title);

private:
    Conference* createConference(uint32_t conferenceNum, const ConferenceId& conferenceId);

    Profile& profile;
    Core* core = nullptr;
    Settings& settings;
    FriendList& friendList;
    ConferenceList& conferenceList;
    IDialogsManager* dialogsManager;

    std::unique_ptr<MessageProcessor::SharedParams> sharedMessageProcessorParams;

    QMap<ToxPk, std::shared_ptr<FriendMessageDispatcher>> friendMessageDispatchers;
    QMap<ToxPk, std::shared_ptr<ChatHistory>> friendChatLogs;
    QMap<ToxPk, std::shared_ptr<FriendChatroom>> friendChatRooms;
    std::map<ToxPk, std::unique_ptr<FriendChatState>> friendChatStates;

    QMap<ConferenceId, std::shared_ptr<ConferenceMessageDispatcher>> conferenceMessageDispatchers;
    QMap<ConferenceId, std::shared_ptr<IChatLog>> conferenceLogs;
    QMap<ConferenceId, std::shared_ptr<ConferenceRoom>> conferenceRooms;
};
