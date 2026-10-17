/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 * Copyright © 2014-2019 by The qTox Project Contributors
 */

#include "chatmanager.h"

#include "src/conferencelist.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/chathistory.h"
#include "src/model/chatroom/conferenceroom.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/chatroom/grouproom.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

#include <QCoreApplication>

#include <cassert>
#include <limits>

ChatManager::ChatManager(Profile& profile_, Settings& settings_, FriendList& friendList_,
                         ConferenceList& conferenceList_, GroupList& groupList_,
                         IDialogsManager* dialogsManager_, QObject* parent)
    : QObject(parent)
    , profile(profile_)
    , settings(settings_)
    , friendList(friendList_)
    , conferenceList(conferenceList_)
    , groupList(groupList_)
    , dialogsManager(dialogsManager_)
    , sharedMessageProcessorParams(
          std::make_unique<MessageProcessor::SharedParams>(Core::getMaxMessageSize()))
{
}

void ChatManager::connectToCore(Core& core_)
{
    core = &core_;

    sharedMessageProcessorParams->setPublicKey(core->getSelfPublicKey().toString());

    connect(core, &Core::friendAdded, this, &ChatManager::onFriendAdded);
    connect(core, &Core::friendStatusChanged, this, &ChatManager::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, this, &ChatManager::onFriendStatusMessageChanged);
    connect(core, &Core::friendUsernameChanged, this, &ChatManager::onFriendUsernameChanged);
    connect(core, &Core::friendMessageReceived, this, &ChatManager::onFriendMessageReceived);
    connect(core, &Core::receiptReceived, this, &ChatManager::onReceiptReceived);
    connect(core, &Core::conferenceMessageReceived, this, &ChatManager::onConferenceMessageReceived);
    connect(core, &Core::emptyConferenceCreated, this, &ChatManager::onEmptyConferenceCreated);
    connect(core, &Core::conferenceJoined, this, &ChatManager::onConferenceJoined);
    connect(core, &Core::conferencePeerlistChanged, this, &ChatManager::onConferencePeerlistChanged);
    connect(core, &Core::conferencePeerNameChanged, this, &ChatManager::onConferencePeerNameChanged);
    connect(core, &Core::conferenceTitleChanged, this, &ChatManager::onConferenceTitleChanged);
    connect(core, &Core::groupMessageReceived, this, &ChatManager::onGroupMessageReceived);
    connect(core, &Core::groupPrivateMessageReceived, this, &ChatManager::onGroupPrivateMessageReceived);
    connect(core, &Core::emptyGroupCreated, this, &ChatManager::onEmptyGroupCreated);
    connect(core, &Core::groupJoined, this, &ChatManager::onGroupJoined);
    connect(core, &Core::groupPeerJoined, this, &ChatManager::onGroupPeerJoined);
    connect(core, &Core::groupPeerExited, this, &ChatManager::onGroupPeerExited);
    connect(core, &Core::groupPeerNameChanged, this, &ChatManager::onGroupPeerNameChanged);
    connect(core, &Core::groupPeerStatusChanged, this, &ChatManager::onGroupPeerStatusChanged);
    connect(core, &Core::groupTopicChanged, this, &ChatManager::onGroupTopicChanged);
    connect(core, &Core::groupSelfJoined, this, &ChatManager::onGroupSelfJoined);
    connect(core, &Core::groupSelfDisconnected, this, &ChatManager::onGroupSelfDisconnected);
    connect(core, &Core::groupJoinFailed, this, &ChatManager::onGroupJoinFailed);
    connect(core, &Core::groupPeerRolesChanged, this, &ChatManager::onGroupPeerRolesChanged);
    connect(core, &Core::groupPasswordChanged, this, &ChatManager::onGroupPasswordChanged);
    connect(core, &Core::groupPeerLimitChanged, this, &ChatManager::onGroupPeerLimitChanged);
    connect(core, &Core::groupTopicLockChanged, this, &ChatManager::onGroupTopicLockChanged);
    connect(core, &Core::groupVoiceStateChanged, this, &ChatManager::onGroupVoiceStateChanged);
    connect(core, &Core::groupPrivacyStateChanged, this, &ChatManager::onGroupPrivacyStateChanged);
}

FriendMessageDispatcher* ChatManager::getFriendDispatcher(const ToxPk& friendPk) const
{
    auto it = friendMessageDispatchers.find(friendPk);
    if (it == friendMessageDispatchers.end()) {
        return nullptr;
    }
    return it->get();
}

ChatHistory* ChatManager::getFriendChatLog(const ToxPk& friendPk) const
{
    auto it = friendChatLogs.find(friendPk);
    if (it == friendChatLogs.end()) {
        return nullptr;
    }
    return it->get();
}

std::shared_ptr<FriendChatroom> ChatManager::getFriendChatroom(const ToxPk& friendPk) const
{
    auto it = friendChatRooms.find(friendPk);
    if (it == friendChatRooms.end()) {
        return nullptr;
    }
    return *it;
}

ConferenceMessageDispatcher* ChatManager::getConferenceDispatcher(const ConferenceId& id) const
{
    auto it = conferenceMessageDispatchers.find(id);
    if (it == conferenceMessageDispatchers.end()) {
        return nullptr;
    }
    return it->get();
}

IChatLog* ChatManager::getConferenceChatLog(const ConferenceId& id) const
{
    auto it = conferenceLogs.find(id);
    if (it == conferenceLogs.end()) {
        return nullptr;
    }
    return it->get();
}

std::shared_ptr<ConferenceRoom> ChatManager::getConferenceRoom(const ConferenceId& id) const
{
    auto it = conferenceRooms.find(id);
    if (it == conferenceRooms.end()) {
        return nullptr;
    }
    return *it;
}

GroupMessageDispatcher* ChatManager::getGroupDispatcher(const GroupId& groupId) const
{
    auto it = groupMessageDispatchers.find(groupId);
    if (it == groupMessageDispatchers.end()) {
        return nullptr;
    }
    return it->get();
}

IChatLog* ChatManager::getGroupChatLog(const GroupId& groupId) const
{
    auto it = groupLogs.find(groupId);
    if (it == groupLogs.end()) {
        return nullptr;
    }
    return it->get();
}

std::shared_ptr<GroupRoom> ChatManager::getGroupRoom(const GroupId& groupId) const
{
    auto it = groupRooms.find(groupId);
    if (it == groupRooms.end()) {
        return nullptr;
    }
    return *it;
}

MessageProcessor::SharedParams& ChatManager::getSharedMessageProcessorParams()
{
    return *sharedMessageProcessorParams;
}

void ChatManager::removeFriend(const ToxPk& friendPk)
{
    Friend* f = friendList.findFriend(friendPk);
    if (f == nullptr) {
        return;
    }

    core->removeFriend(f->getId());

    // Aliases aren't supported for non-friend peers in conferences, revert to basic username.
    for (Conference* c : conferenceList.getAllConferences()) {
        if (c->getPeerList().contains(friendPk)) {
            c->updateUsername(friendPk, f->getUserName());
        }
    }

    friendMessageDispatchers.remove(friendPk);
    friendChatLogs.remove(friendPk);
    friendChatRooms.remove(friendPk);
}

void ChatManager::removeConference(const ConferenceId& conferenceId)
{
    Conference* c = conferenceList.findConference(conferenceId);
    if (c == nullptr) {
        return;
    }

    core->removeConference(c->getId());

    conferenceMessageDispatchers.remove(conferenceId);
    conferenceLogs.remove(conferenceId);
    conferenceRooms.remove(conferenceId);
}

void ChatManager::removeFriendModel(const ToxPk& friendPk)
{
    friendMessageDispatchers.remove(friendPk);
    friendChatLogs.remove(friendPk);
    friendChatRooms.remove(friendPk);
}

void ChatManager::removeConferenceModel(const ConferenceId& conferenceId)
{
    conferenceMessageDispatchers.remove(conferenceId);
    conferenceLogs.remove(conferenceId);
    conferenceRooms.remove(conferenceId);
}

void ChatManager::removeGroup(const GroupId& groupId)
{
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    core->quitGroup(g->getId());

    groupMessageDispatchers.remove(groupId);
    groupLogs.remove(groupId);
    groupRooms.remove(groupId);
    settings.removeSavedGroup(groupId.toString());
}

void ChatManager::removeGroupModel(const GroupId& groupId)
{
    groupMessageDispatchers.remove(groupId);
    groupLogs.remove(groupId);
    groupRooms.remove(groupId);
}

void ChatManager::onFriendAdded(uint32_t friendId, const ToxPk& friendPk)
{
    assert(core != nullptr);
    settings.updateFriendAddress(friendPk.toString());

    Friend* newFriend = friendList.addFriend(friendId, friendPk, settings);
    auto chatroom =
        std::make_shared<FriendChatroom>(newFriend, dialogsManager, *core, settings, conferenceList,
                                         groupList);
    auto friendMessageDispatcher =
        std::make_shared<FriendMessageDispatcher>(*newFriend,
                                                  MessageProcessor(*sharedMessageProcessorParams),
                                                  *core);

    auto* history = profile.getHistory();
    auto chatHistory =
        std::make_shared<ChatHistory>(*newFriend, history, *core, settings,
                                      *friendMessageDispatcher, friendList, conferenceList, groupList);

    friendMessageDispatchers[friendPk] = friendMessageDispatcher;
    friendChatLogs[friendPk] = chatHistory;
    friendChatRooms[friendPk] = chatroom;

    emit friendAdded(newFriend, chatroom, friendMessageDispatcher, chatHistory);
}

void ChatManager::onFriendStatusChanged(uint32_t friendId, Status::Status status)
{
    const auto& friendPk = friendList.id2Key(friendId);
    Friend* f = friendList.findFriend(friendPk);
    if (f == nullptr) {
        return;
    }

    f->setStatus(status);
}

void ChatManager::onFriendStatusMessageChanged(uint32_t friendId, const QString& message)
{
    const auto& friendPk = friendList.id2Key(friendId);
    Friend* f = friendList.findFriend(friendPk);
    if (f == nullptr) {
        return;
    }

    QString str = message;
    str.replace('\n', ' ').remove('\r').remove(QChar('\0'));
    f->setStatusMessage(str);
}

void ChatManager::onFriendUsernameChanged(uint32_t friendId, const QString& username)
{
    const auto& friendPk = friendList.id2Key(friendId);
    Friend* f = friendList.findFriend(friendPk);
    if (f == nullptr) {
        return;
    }

    QString str = username;
    str.replace('\n', ' ').remove('\r').remove(QChar('\0'));
    f->setName(str);
}

void ChatManager::onFriendMessageReceived(uint32_t friendId, const QString& message, bool isAction)
{
    const auto& friendKey = friendList.id2Key(friendId);
    Friend* f = friendList.findFriend(friendKey);
    if (f == nullptr) {
        return;
    }

    friendMessageDispatchers[f->getPublicKey()]->onMessageReceived(isAction, message);
}

void ChatManager::onReceiptReceived(uint32_t friendId, ReceiptNum receipt)
{
    const auto& friendKey = friendList.id2Key(friendId);
    Friend* f = friendList.findFriend(friendKey);
    if (f == nullptr) {
        return;
    }

    friendMessageDispatchers[f->getPublicKey()]->onReceiptReceived(receipt);
}

void ChatManager::onConferenceMessageReceived(uint32_t conferencenumber, uint32_t peernumber,
                                              const QString& message, bool isAction)
{
    const ConferenceId& conferenceId = conferenceList.id2Key(conferencenumber);
    assert(conferenceList.findConference(conferenceId));

    const ToxPk author = core->getConferencePeerPk(conferencenumber, peernumber);

    conferenceMessageDispatchers[conferenceId]->onMessageReceived(author, isAction, message);
}

void ChatManager::onEmptyConferenceCreated(uint32_t conferenceNum, const ConferenceId& id,
                                           const QString& title)
{
    Conference* conference = createConference(conferenceNum, id);
    if (conference == nullptr) {
        return;
    }
    if (title.isEmpty()) {
        emit conferenceNeedsName(id);
    } else {
        conference->setTitle(QString(), title);
    }
}

void ChatManager::onConferenceJoined(uint32_t conferenceNum, const ConferenceId& id)
{
    createConference(conferenceNum, id);
}

void ChatManager::onConferencePeerlistChanged(uint32_t conferencenumber)
{
    const ConferenceId& conferenceId = conferenceList.id2Key(conferencenumber);
    Conference* c = conferenceList.findConference(conferenceId);
    assert(c);
    c->regeneratePeerList();
}

void ChatManager::onConferencePeerNameChanged(uint32_t conferencenumber, const ToxPk& peerPk,
                                              const QString& newName)
{
    const ConferenceId& conferenceId = conferenceList.id2Key(conferencenumber);
    Conference* c = conferenceList.findConference(conferenceId);
    assert(c);

    const QString setName = friendList.decideNickname(peerPk, newName);
    c->updateUsername(peerPk, newName);
}

void ChatManager::onConferenceTitleChanged(uint32_t conferencenumber, const QString& author,
                                           const QString& title)
{
    const ConferenceId& conferenceId = conferenceList.id2Key(conferencenumber);
    Conference* c = conferenceList.findConference(conferenceId);
    assert(c);

    c->setTitle(author, title);
}

void ChatManager::onGroupMessageReceived(uint32_t groupNumber, uint32_t peerId, const QString& message,
                                         bool isAction)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    const ToxPk author = core->getGroupPeerPk(groupNumber, peerId);

    groupMessageDispatchers[groupId]->onMessageReceived(author, isAction, message);
}

void ChatManager::onGroupPrivateMessageReceived(uint32_t groupNumber, uint32_t peerId,
                                                 const QString& message, bool isAction)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    const ToxPk author = core->getGroupPeerPk(groupNumber, peerId);

    groupMessageDispatchers[groupId]->onPrivateMessageReceived(author, isAction, message);
}

void ChatManager::onEmptyGroupCreated(uint32_t groupNumber, const GroupId& groupId,
                                      const QString& groupName)
{
    Group* group = createGroup(groupNumber, groupId, QString());
    if (group == nullptr) {
        return;
    }
    if (!groupId.isEmpty()) {
        settings.addSavedGroup(groupId.toString());
        if (!groupName.isEmpty()) {
            settings.setGroupName(groupId.toString(), groupName);
            group->setName(groupName);
        }
    }
    addSelfToGroup(group);
}

void ChatManager::onGroupJoined(uint32_t groupNumber, const GroupId& groupId)
{
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        const QString groupName = core->getGroupTitle(groupNumber);
        g = createGroup(groupNumber, groupId, groupName);
    } else {
        updateGroupNumber(g, groupNumber);
    }
    if (g != nullptr) {
        addSelfToGroup(g);
    }
    if (!groupId.isEmpty()) {
        settings.addSavedGroup(groupId.toString());
    }
}

void ChatManager::onGroupPeerJoined(uint32_t groupNumber, uint32_t peerId)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    g->onPeerJoin(peerId);
}

void ChatManager::onGroupPeerExited(uint32_t groupNumber, uint32_t peerId)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    g->onPeerExit(peerId);
}

void ChatManager::onGroupPeerNameChanged(uint32_t groupNumber, uint32_t peerId, const QString& newName)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    g->onPeerNameChanged(peerId, newName);
}

void ChatManager::onGroupPeerStatusChanged(uint32_t groupNumber, uint32_t peerId, Status::Status status)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    g->onPeerStatusChanged(peerId, status);
}

void ChatManager::onGroupTopicChanged(uint32_t groupNumber, const QString& topic)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        return;
    }

    g->setTopic(QString(), topic);
    settings.setGroupTopic(groupId.toString(), topic);
}

void ChatManager::onGroupSelfJoined(uint32_t groupNumber)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g == nullptr) {
        const GroupId persistentId = core->getGroupPersistentId(groupNumber);
        if (!persistentId.isEmpty()) {
            g = groupList.findGroup(persistentId);
        }
        if (g == nullptr) {
            const QString groupName = core->getGroupTitle(groupNumber);
            g = createGroup(groupNumber, persistentId, groupName);
        }
    }
    if (g != nullptr) {
        updateGroupNumber(g, groupNumber);
        addSelfToGroup(g);
        g->updatePeerRoles();
        const QString groupName = core->getGroupTitle(groupNumber);
        if (!groupName.isEmpty()) {
            g->updateName(groupName);
        }
        const QString alias = settings.getGroupName(g->getPersistentId().toString());
        if (!alias.isEmpty() && alias != g->getName()) {
            g->setName(alias);
        }
        const QString nickname = settings.getGroupNickname(g->getPersistentId().toString());
        if (!nickname.isEmpty()) {
            g->setGroupNickname(nickname);
        }
        const QString groupTopic = core->getGroupTopic(groupNumber);
        if (!groupTopic.isEmpty()) {
            g->setTopic(QString(), groupTopic);
            settings.setGroupTopic(g->getPersistentId().toString(), groupTopic);
        }
    }
}

void ChatManager::addSelfToGroup(Group* g)
{
    const uint32_t groupNumber = g->getId();
    const uint32_t selfPeerId = core->getGroupSelfPeerId(groupNumber);
    if (selfPeerId == std::numeric_limits<uint32_t>::max()) {
        return;
    }
    g->onPeerJoin(selfPeerId);
}

void ChatManager::updateGroupNumber(Group* g, uint32_t groupNumber)
{
    if (g->getId() != groupNumber) {
        groupList.setToxGroupNum(g->getId(), groupNumber, g->getPersistentId());
        g->setToxGroupNumber(groupNumber);
    }
}

void ChatManager::onGroupSelfDisconnected(uint32_t groupNumber)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        g->clearPeers();
    }
}

void ChatManager::onGroupJoinFailed(uint32_t groupNumber, Tox_Group_Join_Fail failType)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        if (failType == TOX_GROUP_JOIN_FAIL_INVALID_PASSWORD) {
            core->quitGroup(g->getId());
            settings.removeSavedGroup(groupId.toString());
            // The UI must be torn down before the model, otherwise the GroupForm
            // keeps a dangling reference to the chat log.
            emit groupRemoved(groupId);
        } else {
            qWarning() << "Group" << groupId.toString() << "join failed temporarily, keeping saved";
        }
    }
}

void ChatManager::onGroupPeerRolesChanged(uint32_t groupNumber)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        g->updatePeerRoles();
    }
}

void ChatManager::onGroupPasswordChanged(uint32_t groupNumber, bool hasPassword)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        g->setPasswordSet(hasPassword);
    }
}

void ChatManager::onGroupPeerLimitChanged(uint32_t groupNumber, uint16_t peerLimit)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        g->setPeerLimit(peerLimit);
    }
}

void ChatManager::onGroupTopicLockChanged(uint32_t groupNumber, GroupTopicLock topicLock)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        g->setTopicLock(topicLock);
    }
}

void ChatManager::onGroupVoiceStateChanged(uint32_t groupNumber, GroupVoiceState voiceState)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        g->setVoiceState(voiceState);
    }
}

void ChatManager::onGroupPrivacyStateChanged(uint32_t groupNumber, GroupPrivacyState privacyState)
{
    const GroupId& groupId = groupList.id2Key(groupNumber);
    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        g->setPrivacyState(privacyState);
    }
}

Conference* ChatManager::createConference(uint32_t conferencenumber, const ConferenceId& conferenceId)
{
    assert(core != nullptr);

    Conference* c = conferenceList.findConference(conferenceId);
    if (c != nullptr) {
        qWarning() << "Conference already exists";
        return c;
    }

    const auto conferenceName =
        QCoreApplication::translate("ChatManager", "Conference #%1").arg(conferencenumber);
    const bool enabled = core->getConferenceAvEnabled(conferencenumber);
    Conference* newConference =
        conferenceList.addConference(*core, conferencenumber, conferenceId, conferenceName, enabled,
                                     core->getUsername(), friendList);
    assert(newConference);

    if (enabled) {
        connect(newConference, &Conference::userLeft, this, [this, newConference](const ToxPk& user) {
            CoreAV* av = core->getAv();
            assert(av);
            av->invalidateConferenceCallPeerSource(*newConference, user);
        });
    }

    auto chatroom = std::make_shared<ConferenceRoom>(newConference, dialogsManager, *core, friendList);
    auto messageDispatcher =
        std::make_shared<ConferenceMessageDispatcher>(*newConference,
                                                      MessageProcessor(*sharedMessageProcessorParams),
                                                      *core, *core, settings);

    auto* history = profile.getHistory();
    auto chatHistory = std::make_shared<ChatHistory>(*newConference, history, *core, settings,
                                                     *messageDispatcher, friendList, conferenceList,
                                                     groupList);

    connect(core, &Core::usernameSet, newConference, &Conference::setSelfName);

    conferenceMessageDispatchers[conferenceId] = messageDispatcher;
    conferenceLogs[conferenceId] = chatHistory;
    conferenceRooms[conferenceId] = chatroom;

    emit conferenceAdded(newConference, chatroom, messageDispatcher, chatHistory);

    return newConference;
}

Group* ChatManager::createGroup(uint32_t groupNumber, const GroupId& groupId, const QString& groupName)
{
    assert(core != nullptr);

    QString name = groupName;

    Group* g = groupList.findGroup(groupId);
    if (g != nullptr) {
        qWarning() << "Group already exists";
        return g;
    }

    Group* newGroup = groupList.addGroup(*core, groupNumber, groupId, name,
                                         core->getUsername(), friendList);
    assert(newGroup);

    newGroup->setPasswordSet(core->getGroupHasPassword(groupNumber));
    newGroup->setPeerLimit(core->getGroupPeerLimit(groupNumber));
    QString topic = core->getGroupTopic(groupNumber);
    if (topic.isEmpty()) {
        topic = settings.getGroupTopic(groupId.toString());
    }
    newGroup->setTopic(QString(), topic);
    newGroup->setTopicLock(core->getGroupTopicLock(groupNumber));
    newGroup->setVoiceState(core->getGroupVoiceState(groupNumber));
    newGroup->setPrivacyState(core->getGroupPrivacyState(groupNumber));

    auto chatroom = std::make_shared<GroupRoom>(newGroup, dialogsManager, *core, friendList);
    auto messageDispatcher =
        std::make_shared<GroupMessageDispatcher>(*newGroup,
                                                 MessageProcessor(*sharedMessageProcessorParams),
                                                 *core, *core, settings);

    auto* history = profile.getHistory();
    auto chatHistory = std::make_shared<ChatHistory>(*newGroup, history, *core, settings,
                                                     *messageDispatcher, friendList, conferenceList,
                                                     groupList);

    connect(core, &Core::usernameSet, newGroup, &Group::setSelfName);

    groupMessageDispatchers[groupId] = messageDispatcher;
    groupLogs[groupId] = chatHistory;
    groupRooms[groupId] = chatroom;

    emit groupAdded(newGroup, chatroom, messageDispatcher, chatHistory);

    return newGroup;
}
