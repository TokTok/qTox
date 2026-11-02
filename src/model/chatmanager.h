/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 * Copyright © 2014-2019 by The qTox Project Contributors
 */

#pragma once

#include <tox/tox.h>

#include "src/core/conferenceid.h"
#include "src/core/groupid.h"
#include "src/core/icoregroupquery.h"
#include "src/core/receiptnum.h"
#include "src/core/toxpk.h"
#include "src/model/conferencemessagedispatcher.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/groupmessagedispatcher.h"
#include "src/model/message.h"

#include <QMap>
#include <QObject>

#include <memory>

class ChatHistory;
class Conference;
class ConferenceList;
class ConferenceRoom;
class Core;
class Friend;
class FriendChatroom;
class FriendList;
class Group;
class GroupList;
class GroupRoom;
class IChatLog;
class IDialogsManager;
class Profile;
class Settings;

class ChatManager : public QObject
{
    Q_OBJECT

public:
    ChatManager(Profile& profile, Settings& settings, FriendList& friendList,
                ConferenceList& conferenceList, GroupList& groupList,
                IDialogsManager* dialogsManager, QObject* parent = nullptr);

    void connectToCore(Core& core);

    FriendMessageDispatcher* getFriendDispatcher(const ToxPk& friendPk) const;
    ChatHistory* getFriendChatLog(const ToxPk& friendPk) const;
    std::shared_ptr<FriendChatroom> getFriendChatroom(const ToxPk& friendPk) const;
    ConferenceMessageDispatcher* getConferenceDispatcher(const ConferenceId& id) const;
    IChatLog* getConferenceChatLog(const ConferenceId& id) const;
    std::shared_ptr<ConferenceRoom> getConferenceRoom(const ConferenceId& id) const;
    GroupMessageDispatcher* getGroupDispatcher(const GroupId& groupId) const;
    IChatLog* getGroupChatLog(const GroupId& groupId) const;
    std::shared_ptr<GroupRoom> getGroupRoom(const GroupId& groupId) const;

    MessageProcessor::SharedParams& getSharedMessageProcessorParams();

    void removeFriend(const ToxPk& friendPk);
    void removeConference(const ConferenceId& conferenceId);
    void removeFriendModel(const ToxPk& friendPk);
    void removeConferenceModel(const ConferenceId& conferenceId);
    void removeGroup(const GroupId& groupId);
    void removeGroupModel(const GroupId& groupId);

signals:
    void friendAdded(Friend* f, std::shared_ptr<FriendChatroom> chatroom,
                     std::shared_ptr<FriendMessageDispatcher> dispatcher,
                     std::shared_ptr<ChatHistory> chatLog);
    void friendRemoved(const ToxPk& friendPk);
    void conferenceAdded(Conference* c, std::shared_ptr<ConferenceRoom> chatroom,
                         std::shared_ptr<ConferenceMessageDispatcher> dispatcher,
                         std::shared_ptr<IChatLog> chatLog);
    void conferenceRemoved(const ConferenceId& conferenceId);
    void conferenceNeedsName(const ConferenceId& conferenceId);
    void groupAdded(Group* g, std::shared_ptr<GroupRoom> chatroom,
                    std::shared_ptr<GroupMessageDispatcher> dispatcher,
                    std::shared_ptr<IChatLog> chatLog);
    void groupRemoved(const GroupId& groupId);

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

    void onGroupMessageReceived(uint32_t groupNumber, uint32_t peerId, const QString& message,
                                bool isAction);
    void onGroupPrivateMessageReceived(uint32_t groupNumber, uint32_t peerId, const QString& message,
                                        bool isAction);
    void onEmptyGroupCreated(uint32_t groupNumber, const GroupId& groupId, const QString& groupName);
    void onGroupJoined(uint32_t groupNumber, const GroupId& groupId);
    void onGroupPeerJoined(uint32_t groupNumber, uint32_t peerId);
    void onGroupPeerExited(uint32_t groupNumber, uint32_t peerId);
    void onGroupPeerNameChanged(uint32_t groupNumber, uint32_t peerId, const QString& newName);
    void onGroupPeerStatusChanged(uint32_t groupNumber, uint32_t peerId, Status::Status status);
    void onGroupTopicChanged(uint32_t groupNumber, const QString& topic);
    void onGroupSelfJoined(uint32_t groupNumber);
    void onGroupSelfDisconnected(uint32_t groupNumber);
    void onGroupJoinFailed(uint32_t groupNumber, Tox_Group_Join_Fail failType);
    void onGroupPeerRolesChanged(uint32_t groupNumber);
    void onGroupPasswordChanged(uint32_t groupNumber, bool hasPassword);
    void onGroupPeerLimitChanged(uint32_t groupNumber, uint16_t peerLimit);
    void onGroupTopicLockChanged(uint32_t groupNumber, GroupTopicLock topicLock);
    void onGroupVoiceStateChanged(uint32_t groupNumber, GroupVoiceState voiceState);
    void onGroupPrivacyStateChanged(uint32_t groupNumber, GroupPrivacyState privacyState);

private:
    Conference* createConference(uint32_t conferenceNum, const ConferenceId& conferenceId);
    Group* createGroup(uint32_t groupNumber, const GroupId& groupId, const QString& groupName);
    void addSelfToGroup(Group* g);
    void updateGroupNumber(Group* g, uint32_t groupNumber);

    Profile& profile;
    Core* core = nullptr;
    Settings& settings;
    FriendList& friendList;
    ConferenceList& conferenceList;
    GroupList& groupList;
    IDialogsManager* dialogsManager;

    std::unique_ptr<MessageProcessor::SharedParams> sharedMessageProcessorParams;

    QMap<ToxPk, std::shared_ptr<FriendMessageDispatcher>> friendMessageDispatchers;
    QMap<ToxPk, std::shared_ptr<ChatHistory>> friendChatLogs;
    QMap<ToxPk, std::shared_ptr<FriendChatroom>> friendChatRooms;

    QMap<ConferenceId, std::shared_ptr<ConferenceMessageDispatcher>> conferenceMessageDispatchers;
    QMap<ConferenceId, std::shared_ptr<IChatLog>> conferenceLogs;
    QMap<ConferenceId, std::shared_ptr<ConferenceRoom>> conferenceRooms;

    QMap<GroupId, std::shared_ptr<GroupMessageDispatcher>> groupMessageDispatchers;
    QMap<GroupId, std::shared_ptr<IChatLog>> groupLogs;
    QMap<GroupId, std::shared_ptr<GroupRoom>> groupRooms;
};
