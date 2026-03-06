/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 * Copyright © 2014-2019 by The qTox Project Contributors
 */

#include "chatmanager.h"

#include "src/conferencelist.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/friendlist.h"
#include "src/model/chathistory.h"
#include "src/model/chatroom/conferenceroom.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/model/friendchatstate.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

#include <QCoreApplication>

#include <cassert>

ChatManager::ChatManager(Profile& profile_, Settings& settings_, FriendList& friendList_,
                         ConferenceList& conferenceList_, IDialogsManager* dialogsManager_,
                         QObject* parent)
    : QObject(parent)
    , profile(profile_)
    , settings(settings_)
    , friendList(friendList_)
    , conferenceList(conferenceList_)
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

FriendChatState* ChatManager::getFriendChatState(const ToxPk& friendPk) const
{
    auto it = friendChatStates.find(friendPk);
    if (it == friendChatStates.end()) {
        return nullptr;
    }
    return it->second.get();
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
    friendChatStates.erase(friendPk);
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
    friendChatStates.erase(friendPk);
}

void ChatManager::removeConferenceModel(const ConferenceId& conferenceId)
{
    conferenceMessageDispatchers.remove(conferenceId);
    conferenceLogs.remove(conferenceId);
    conferenceRooms.remove(conferenceId);
}

void ChatManager::onFriendAdded(uint32_t friendId, const ToxPk& friendPk)
{
    assert(core != nullptr);
    settings.updateFriendAddress(friendPk.toString());

    Friend* newFriend = friendList.addFriend(friendId, friendPk, settings);
    auto chatroom =
        std::make_shared<FriendChatroom>(newFriend, dialogsManager, *core, settings, conferenceList);
    auto friendMessageDispatcher =
        std::make_shared<FriendMessageDispatcher>(*newFriend,
                                                  MessageProcessor(*sharedMessageProcessorParams),
                                                  *core);

    auto* history = profile.getHistory();
    auto chatHistory =
        std::make_shared<ChatHistory>(*newFriend, history, *core, settings,
                                      *friendMessageDispatcher, friendList, conferenceList);

    CoreAV* av = core->getAv();
    auto chatState =
        std::make_unique<FriendChatState>(*newFriend, *av, *chatHistory, settings, settings, this);

    connect(av, &CoreAV::avInvite, chatState.get(), &FriendChatState::onAvInvite);
    connect(av, &CoreAV::avStart, chatState.get(), &FriendChatState::onAvStart);
    connect(av, &CoreAV::avEnd, chatState.get(), &FriendChatState::onAvEnd);
    connect(newFriend, &Friend::statusChanged, chatState.get(),
            &FriendChatState::onFriendStatusChanged);
    connect(core, &Core::friendTypingChanged, chatState.get(), &FriendChatState::onFriendTypingChanged);
    connect(chatState.get(), &FriendChatState::sendTypingRequested, core, &Core::sendTyping);

    FriendChatState* chatStatePtr = chatState.get();

    friendMessageDispatchers[friendPk] = friendMessageDispatcher;
    friendChatLogs[friendPk] = chatHistory;
    friendChatRooms[friendPk] = chatroom;
    friendChatStates[friendPk] = std::move(chatState);

    emit friendAdded(newFriend, chatroom, friendMessageDispatcher, chatHistory, chatStatePtr);
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
                                                     *messageDispatcher, friendList, conferenceList);

    connect(core, &Core::usernameSet, newConference, &Conference::setSelfName);

    conferenceMessageDispatchers[conferenceId] = messageDispatcher;
    conferenceLogs[conferenceId] = chatHistory;
    conferenceRooms[conferenceId] = chatroom;

    emit conferenceAdded(newConference, chatroom, messageDispatcher, chatHistory);

    return newConference;
}
