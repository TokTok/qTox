/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "core.h"

#include "coreav.h"
#include "corefile.h"

#include "src/core/dhtserver.h"
#include "src/core/icoresettings.h"
#include "src/core/toxoptions.h"
#include "src/core/toxstring.h"
#include "src/model/conferenceinvite.h"
#include "src/model/groupinvite.h"
#include "src/model/ibootstraplistgenerator.h"
#include "src/model/status.h"
#include "util/toxcoreerrorparser.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringBuilder>
#include <QTimer>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>
#include <random>
#include <tox/tox.h>

const QString Core::TOX_EXT = ".tox";

// how often to force a group reconnect (workaround for toxcore not restoring
// group connections/roles after a restart)
static constexpr int GROUP_RECONNECT_INTERVAL_MS = 10000;

#define ASSERT_CORE_THREAD assert(QThread::currentThread() == coreThread.get())

namespace {

QList<DhtServer> shuffleBootstrapNodes(QList<DhtServer> bootstrapNodes)
{
    std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::shuffle(bootstrapNodes.begin(), bootstrapNodes.end(), rng);
    return bootstrapNodes;
}

} // namespace

Core::Core(QThread* coreThread_, IBootstrapListGenerator& bootstrapListGenerator_,
           const ICoreSettings& settings_)
    : tox(nullptr)
    , toxTimer{new QTimer{this}}
    , coreThread(coreThread_)
    , bootstrapListGenerator(bootstrapListGenerator_)
    , settings(settings_)
{
    assert(toxTimer);
    // need to migrate Settings and History if this changes
    assert(TOX_PUBLIC_KEY_SIZE == tox_public_key_size());
    assert(TOX_CONFERENCE_ID_SIZE == tox_conference_id_size());
    assert(TOX_GROUP_CHAT_ID_SIZE == tox_group_chat_id_size());
    assert(TOX_ADDRESS_SIZE == tox_address_size());
    toxTimer->setSingleShot(true);
    connect(toxTimer, &QTimer::timeout, this, &Core::process);
    connect(coreThread_, &QThread::finished, toxTimer, &QTimer::stop);
}

Core::~Core()
{
    /*
     * First stop the thread to stop the timer and avoid Core emitting callbacks
     * into an already destructed CoreAV.
     */
    coreThread->exit(0);
    coreThread->wait();

    tox.reset();
}

/**
 * @brief Registers all toxcore callbacks
 * @param tox Tox instance to register the callbacks on
 */
void Core::registerCallbacks(Tox* tox)
{
    tox_callback_friend_request(tox, onFriendRequest);
    tox_callback_friend_message(tox, onFriendMessage);
    tox_callback_friend_name(tox, onFriendNameChange);
    tox_callback_friend_typing(tox, onFriendTypingChange);
    tox_callback_friend_status_message(tox, onStatusMessageChanged);
    tox_callback_friend_status(tox, onUserStatusChanged);
    tox_callback_friend_connection_status(tox, onConnectionStatusChanged);
    tox_callback_friend_read_receipt(tox, onReadReceiptCallback);
    tox_callback_conference_invite(tox, onConferenceInvite);
    tox_callback_conference_message(tox, onConferenceMessage);
    tox_callback_conference_peer_list_changed(tox, onConferencePeerListChange);
    tox_callback_conference_peer_name(tox, onConferencePeerNameChange);
    tox_callback_conference_title(tox, onConferenceTitleChange);

    tox_callback_group_invite(tox, onGroupInvite);
    tox_callback_group_message(tox, onGroupMessage);
    tox_callback_group_private_message(tox, onGroupPrivateMessage);
    tox_callback_group_peer_join(tox, onGroupPeerJoin);
    tox_callback_group_peer_exit(tox, onGroupPeerExit);
    tox_callback_group_peer_name(tox, onGroupPeerNameChange);
    tox_callback_group_peer_status(tox, onGroupPeerStatusChange);
    tox_callback_group_self_join(tox, onGroupSelfJoin);
    tox_callback_group_topic(tox, onGroupTopic);
    tox_callback_group_join_fail(tox, onGroupJoinFail);
    tox_callback_group_moderation(tox, onGroupModeration);
    tox_callback_group_password(tox, onGroupPassword);
    tox_callback_group_peer_limit(tox, onGroupPeerLimit);
    tox_callback_group_topic_lock(tox, onGroupTopicLock);
    tox_callback_group_voice_state(tox, onGroupVoiceState);
    tox_callback_group_privacy_state(tox, onGroupPrivacyState);
}

/**
 * @brief Factory method for the Core object
 * @param savedata empty if new profile or saved data else
 * @param settings Settings specific to Core
 * @return nullptr or a Core object ready to start
 */
std::pair<ToxCorePtr, Core::ToxCoreErrors> Core::makeToxCore(const QByteArray& savedata,
                                                             const ICoreSettings& settings,
                                                             IBootstrapListGenerator& bootstrapNodes)
{
    auto* thread = new QThread();
    thread->setObjectName("qTox Core");

    auto toxOptions = ToxOptions::makeToxOptions(savedata, settings);
    if (toxOptions == nullptr) {
        qCritical() << "Could not allocate ToxOptions data structure";
        return {nullptr, ToxCoreErrors::ERROR_ALLOC};
    }

    ToxCorePtr core(new Core(thread, bootstrapNodes, settings));

    Tox_Err_New tox_err;
    core->tox = ToxPtr(tox_new(toxOptions->get(), &tox_err));

    switch (tox_err) {
    case TOX_ERR_NEW_OK:
        break;

    case TOX_ERR_NEW_LOAD_BAD_FORMAT:
        qCritical() << "Failed to parse Tox save data";
        return {nullptr, ToxCoreErrors::INVALID_SAVE};

    case TOX_ERR_NEW_PORT_ALLOC:
        if (toxOptions->getIPv6Enabled()) {
            toxOptions->setIPv6Enabled(false);
            core->tox = ToxPtr(tox_new(toxOptions->get(), &tox_err));
            if (tox_err == TOX_ERR_NEW_OK) {
                qWarning() << "Core failed to start with IPv6, falling back to IPv4. LAN discovery "
                              "may not work properly.";
                break;
            }
        }

        qCritical() << "Can't to bind the port";
        return {nullptr, ToxCoreErrors::FAILED_TO_START};

    case TOX_ERR_NEW_PROXY_BAD_HOST:
    case TOX_ERR_NEW_PROXY_BAD_PORT:
    case TOX_ERR_NEW_PROXY_BAD_TYPE:
        qCritical() << "Bad proxy, error code:" << tox_err;
        return {nullptr, ToxCoreErrors::BAD_PROXY};

    case TOX_ERR_NEW_PROXY_NOT_FOUND:
        qCritical() << "Proxy not found";
        return {nullptr, ToxCoreErrors::BAD_PROXY};

    case TOX_ERR_NEW_LOAD_ENCRYPTED:
        qCritical() << "Attempted to load encrypted Tox save data";
        return {nullptr, ToxCoreErrors::INVALID_SAVE};

    case TOX_ERR_NEW_MALLOC:
        qCritical() << "Memory allocation failed";
        return {nullptr, ToxCoreErrors::ERROR_ALLOC};

    case TOX_ERR_NEW_NULL:
        qCritical() << "A parameter was null";
        return {nullptr, ToxCoreErrors::FAILED_TO_START};

    default:
        qCritical() << "Toxcore failed to start, unknown error code:" << tox_err;
        return {nullptr, ToxCoreErrors::FAILED_TO_START};
    }

    // tox should be valid by now
    assert(core->tox != nullptr);

    // create CoreFile
    core->file = CoreFile::makeCoreFile(core.get(), core->tox.get(), core->coreLoopLock);
    if (!core->file) {
        qCritical() << "CoreFile failed to start";
        return {nullptr, ToxCoreErrors::FAILED_TO_START};
    }

    registerCallbacks(core->tox.get());

    // connect the thread with the Core
    connect(thread, &QThread::started, core.get(), &Core::onStarted);
    core->moveToThread(thread);

    // when leaving this function 'core' should be ready for it's start() action or
    // a nullptr
    return {std::move(core), ToxCoreErrors::OK};
}

void Core::onStarted()
{
    ASSERT_CORE_THREAD;

    // One time initialization stuff
    const QString name = getUsername();
    if (!name.isEmpty()) {
        emit usernameSet(name);
    }

    const QString msg = getStatusMessage();
    if (!msg.isEmpty()) {
        emit statusMessageSet(msg);
    }

    const ToxId id = getSelfId();
    // Id comes from toxcore, must be valid
    assert(id.isValid());
    emit idSet(id);

    loadFriends();
    loadConferences();
    loadGroups();

    process(); // starts its own timer
}

/**
 * @brief Starts toxcore and it's event loop, can be called from any thread
 */
void Core::start()
{
    coreThread->start();
}

const CoreAV* Core::getAv() const
{
    return av;
}

CoreAV* Core::getAv()
{
    return av;
}

void Core::setAv(CoreAV* coreAv)
{
    av = coreAv;
}

CoreFile* Core::getCoreFile() const
{
    return file.get();
}

Tox* Core::getTox() const
{
    return tox.get();
}

QRecursiveMutex& Core::getCoreLoopLock() const
{
    return coreLoopLock;
}

/**
 * @brief Processes toxcore events and ensure we stay connected, called by its own timer
 */
void Core::process()
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    ASSERT_CORE_THREAD;

    tox_iterate(tox.get(), this);

#ifdef DEBUG
    // we want to see the debug messages immediately
    fflush(stdout);
#endif

    // TODO(sudden6): recheck if this is still necessary
    if (checkConnection()) {
        tolerance = CORE_DISCONNECT_TOLERANCE;
    } else if ((--tolerance) == 0) {
        bootstrapDht();
        tolerance = 3 * CORE_DISCONNECT_TOLERANCE;
    }

    const unsigned sleeptime =
        qMin(tox_iteration_interval(tox.get()), getCoreFile()->corefileIterationInterval());
    toxTimer->start(sleeptime);
}

bool Core::checkConnection()
{
    ASSERT_CORE_THREAD;
    auto selfConnection = tox_self_get_connection_status(tox.get());
    const char* connectionName = "unknown";
    bool toxConnected = false;
    switch (selfConnection) {
    case TOX_CONNECTION_NONE:
        toxConnected = false;
        break;
    case TOX_CONNECTION_TCP:
        toxConnected = true;
        connectionName = "a TCP relay";
        break;
    case TOX_CONNECTION_UDP:
        toxConnected = true;
        connectionName = "the UDP DHT";
        break;
    default:
        qWarning() << "tox_self_get_connection_status returned unknown enum!";
        break;
    }

    if (toxConnected && !isConnected) {
        qDebug() << "Connected to" << connectionName;
        emit connected();
    } else if (!toxConnected && isConnected) {
        qDebug() << "Disconnected from the DHT";
        emit disconnected();
    }

    isConnected = toxConnected;
    return toxConnected;
}

/**
 * @brief Connects us to the Tox network
 */
void Core::bootstrapDht()
{
    ASSERT_CORE_THREAD;


    const auto shuffledBootstrapNodes =
        shuffleBootstrapNodes(bootstrapListGenerator.getBootstrapNodes());
    if (shuffledBootstrapNodes.empty()) {
        qWarning() << "No bootstrap node list";
        return;
    }

    // i think the more we bootstrap, the more we jitter because the more we overwrite nodes
    auto numNewNodes = 2;
    for (int i = 0; i < numNewNodes && i < shuffledBootstrapNodes.size(); ++i) {
        const auto& dhtServer = shuffledBootstrapNodes.at(i);
        QByteArray address;
        if (!dhtServer.ipv4.isEmpty()) {
            address = dhtServer.ipv4.toLatin1();
        } else if (!dhtServer.ipv6.isEmpty() && settings.getEnableIPv6()) {
            address = dhtServer.ipv6.toLatin1();
        } else {
            ++numNewNodes;
            continue;
        }

        const ToxPk pk{dhtServer.publicKey};
        qDebug() << "Connecting to bootstrap node" << pk.toString();
        const uint8_t* pkPtr = pk.getData();

        Tox_Err_Bootstrap error;
        if (dhtServer.statusUdp) {
            tox_bootstrap(tox.get(), address.constData(), dhtServer.udpPort, pkPtr, &error);
            PARSE_ERR(error);
        }
        if (dhtServer.statusTcp) {
            const auto ports = dhtServer.tcpPorts.size();
            const auto tcpPort = dhtServer.tcpPorts[rand() % ports];
            tox_add_tcp_relay(tox.get(), address.constData(), tcpPort, pkPtr, &error);
            PARSE_ERR(error);
        }
    }
}

void Core::onFriendRequest(Tox* tox, const uint8_t* cFriendPk, const uint8_t* cMessage,
                           size_t cMessageSize, void* core)
{
    std::ignore = tox;
    const ToxPk friendPk(cFriendPk);
    const QString requestMessage = ToxString(cMessage, cMessageSize).getQString();
    emit static_cast<Core*>(core)->friendRequestReceived(friendPk, requestMessage);
}

void Core::onFriendMessage(Tox* tox, uint32_t friendId, Tox_Message_Type type,
                           const uint8_t* cMessage, size_t cMessageSize, void* core)
{
    std::ignore = tox;
    const bool isAction = (type == TOX_MESSAGE_TYPE_ACTION);
    const QString msg = ToxString(cMessage, cMessageSize).getQString();
    emit static_cast<Core*>(core)->friendMessageReceived(friendId, msg, isAction);
}

void Core::onFriendNameChange(Tox* tox, uint32_t friendId, const uint8_t* cName, size_t cNameSize,
                              void* core)
{
    std::ignore = tox;
    const QString newName = ToxString(cName, cNameSize).getQString();
    // no saveRequest, this callback is called on every connection, not just on name change
    emit static_cast<Core*>(core)->friendUsernameChanged(friendId, newName);
}

void Core::onFriendTypingChange(Tox* tox, uint32_t friendId, bool isTyping, void* core)
{
    std::ignore = tox;
    emit static_cast<Core*>(core)->friendTypingChanged(friendId, isTyping);
}

void Core::onStatusMessageChanged(Tox* tox, uint32_t friendId, const uint8_t* cMessage,
                                  size_t cMessageSize, void* core)
{
    std::ignore = tox;
    const QString message = ToxString(cMessage, cMessageSize).getQString();
    // no saveRequest, this callback is called on every connection, not just on name change
    emit static_cast<Core*>(core)->friendStatusMessageChanged(friendId, message);
}

void Core::onUserStatusChanged(Tox* tox, uint32_t friendId, Tox_User_Status userstatus, void* core)
{
    std::ignore = tox;
    Status::Status status;
    switch (userstatus) {
    case TOX_USER_STATUS_AWAY:
        status = Status::Status::Away;
        break;

    case TOX_USER_STATUS_BUSY:
        status = Status::Status::Busy;
        break;

    default:
        status = Status::Status::Online;
        break;
    }

    // no saveRequest, this callback is called on every connection, not just on name change
    emit static_cast<Core*>(core)->friendStatusChanged(friendId, status);
}

void Core::onConnectionStatusChanged(Tox* tox, uint32_t friendId, Tox_Connection status, void* vCore)
{
    std::ignore = tox;
    Core* core = static_cast<Core*>(vCore);
    Status::Status friendStatus = Status::Status::Offline;
    switch (status) {
    case TOX_CONNECTION_NONE:
        friendStatus = Status::Status::Offline;
        qDebug() << "Disconnected from friend" << friendId;
        break;
    case TOX_CONNECTION_TCP:
        friendStatus = Status::Status::Online;
        qDebug() << "Connected to friend" << friendId << "through a TCP relay";
        break;
    case TOX_CONNECTION_UDP:
        friendStatus = Status::Status::Online;
        qDebug() << "Connected to friend" << friendId << "directly with UDP";
        break;
    default:
        qWarning() << "tox_callback_friend_connection_status returned unknown enum!";
        break;
    }

    // Ignore Online because it will be emitted from onUserStatusChanged
    const bool isOffline = friendStatus == Status::Status::Offline;
    if (isOffline) {
        emit core->friendStatusChanged(friendId, friendStatus);
        core->checkLastOnline(friendId);
    }
}

void Core::onConferenceInvite(Tox* tox, uint32_t friendId, Tox_Conference_Type type,
                              const uint8_t* cookie, size_t length, void* vCore)
{
    std::ignore = tox;
    Core* core = static_cast<Core*>(vCore);
    const QByteArray data(reinterpret_cast<const char*>(cookie), length);
    const ConferenceInvite inviteInfo(friendId, type, data);
    switch (type) {
    case TOX_CONFERENCE_TYPE_TEXT:
        qDebug() << "Text conference invite by" << friendId;
        emit core->conferenceInviteReceived(inviteInfo);
        break;

    case TOX_CONFERENCE_TYPE_AV:
        qDebug() << "AV conference invite by" << friendId;
        emit core->conferenceInviteReceived(inviteInfo);
        break;

    default:
        qWarning() << "Conference invite with unknown type" << type;
    }
}

void Core::onConferenceMessage(Tox* tox, uint32_t conferenceId, uint32_t peerId, Tox_Message_Type type,
                               const uint8_t* cMessage, size_t length, void* vCore)
{
    std::ignore = tox;
    Core* core = static_cast<Core*>(vCore);
    const bool isAction = type == TOX_MESSAGE_TYPE_ACTION;
    const QString message = ToxString(cMessage, length).getQString();
    emit core->conferenceMessageReceived(conferenceId, peerId, message, isAction);
}

void Core::onConferencePeerListChange(Tox* tox, uint32_t conferenceId, void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qDebug("Conference %u peerlist changed", conferenceId);
    // no saveRequest, this callback is called on every connection to conference peer, not just on brand new peers
    emit core->conferencePeerlistChanged(conferenceId);
}

void Core::onConferencePeerNameChange(Tox* tox, uint32_t conferenceId, uint32_t peerId,
                                      const uint8_t* name, size_t length, void* vCore)
{
    std::ignore = tox;
    const auto newName = ToxString(name, length).getQString();
    qDebug().nospace() << "Conference " << conferenceId << ", peer " << peerId << ", name " << newName;
    auto* core = static_cast<Core*>(vCore);
    auto peerPk = core->getConferencePeerPk(conferenceId, peerId);
    emit core->conferencePeerNameChanged(conferenceId, peerPk, newName);
}

void Core::onConferenceTitleChange(Tox* tox, uint32_t conferenceId, uint32_t peerId,
                                   const uint8_t* cTitle, size_t length, void* vCore)
{
    std::ignore = tox;
    Core* core = static_cast<Core*>(vCore);
    QString author;
    // from tox.h: "If peer_number == UINT32_MAX, then author is unknown (e.g. initial joining the conference)."
    if (peerId != std::numeric_limits<uint32_t>::max()) {
        author = core->getConferencePeerName(conferenceId, peerId);
    }
    emit core->saveRequest();
    emit core->conferenceTitleChanged(conferenceId, author, ToxString(cTitle, length).getQString());
}


void Core::onGroupInvite(Tox* tox, uint32_t friendId, const uint8_t* inviteData, size_t length,
                         const uint8_t* groupName, size_t groupNameLength, void* vCore)
{
    std::ignore = tox;
    Core* core = static_cast<Core*>(vCore);
    const QByteArray data(reinterpret_cast<const char*>(inviteData), length);
    const QString name = ToxString(groupName, groupNameLength).getQString();
    const GroupInvite inviteInfo(friendId, data, name);
    qDebug() << "Group invite by friend" << friendId << "to group" << name;
    emit core->groupInviteReceived(inviteInfo);
}

void Core::onGroupMessage(Tox* tox, uint32_t groupNumber, uint32_t peerId, Tox_Message_Type type,
                          const uint8_t* cMessage, size_t length, Tox_Group_Message_Id messageId,
                          void* vCore)
{
    std::ignore = tox;
    std::ignore = messageId;
    Core* core = static_cast<Core*>(vCore);
    const bool isAction = type == TOX_MESSAGE_TYPE_ACTION;
    const QString message = ToxString(cMessage, length).getQString();
    emit core->groupMessageReceived(groupNumber, peerId, message, isAction);
}

void Core::onGroupPrivateMessage(Tox* tox, uint32_t groupNumber, uint32_t peerId, Tox_Message_Type type,
                                  const uint8_t* cMessage, size_t length, Tox_Group_Message_Id messageId,
                                  void* vCore)
{
    std::ignore = tox;
    std::ignore = messageId;
    Core* core = static_cast<Core*>(vCore);
    const bool isAction = type == TOX_MESSAGE_TYPE_ACTION;
    const QString message = ToxString(cMessage, length).getQString();
    emit core->groupPrivateMessageReceived(groupNumber, peerId, message, isAction);
}

void Core::onGroupPeerJoin(Tox* tox, uint32_t groupNumber, uint32_t peerId, void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qDebug("Group %u peer %u joined", groupNumber, peerId);
    ++core->groupPeerCounts[groupNumber];
    core->stopGroupReconnectTimer(groupNumber);
    emit core->groupPeerJoined(groupNumber, peerId);
}

void Core::onGroupPeerExit(Tox* tox, uint32_t groupNumber, uint32_t peerId, Tox_Group_Exit_Type exitType,
                           const uint8_t* name, size_t nameLength, const uint8_t* partMessage,
                           size_t partMessageLength, void* vCore)
{
    std::ignore = tox;
    std::ignore = name;
    std::ignore = nameLength;
    std::ignore = partMessage;
    std::ignore = partMessageLength;
    auto* const core = static_cast<Core*>(vCore);
    qDebug("Group %u peer %u left, exit type %d", groupNumber, peerId, static_cast<int>(exitType));
    auto it = core->groupPeerCounts.find(groupNumber);
    if (it != core->groupPeerCounts.end() && it.value() > 0) {
        if (it.value() == 1) {
            core->startGroupReconnectTimer(groupNumber);
            core->groupPeerCounts.erase(it);
        } else {
            --it.value();
        }
    }
    emit core->groupPeerExited(groupNumber, peerId);
}

void Core::onGroupPeerNameChange(Tox* tox, uint32_t groupNumber, uint32_t peerId,
                                 const uint8_t* name, size_t length, void* vCore)
{
    std::ignore = tox;
    const auto newName = ToxString(name, length).getQString();
    qDebug().nospace() << "Group " << groupNumber << ", peer " << peerId << ", name " << newName;
    auto* core = static_cast<Core*>(vCore);
    emit core->groupPeerNameChanged(groupNumber, peerId, newName);
}

void Core::onGroupPeerStatusChange(Tox* tox, uint32_t groupNumber, uint32_t peerId,
                                   Tox_User_Status status, void* vCore)
{
    std::ignore = tox;
    qDebug().nospace() << "Group " << groupNumber << ", peer " << peerId
                       << ", status " << static_cast<int>(status);
    auto* core = static_cast<Core*>(vCore);
    emit core->groupPeerStatusChanged(groupNumber, peerId, static_cast<Status::Status>(status));
}

void Core::onGroupSelfJoin(Tox* tox, uint32_t groupNumber, void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qDebug("Joined group %u", groupNumber);
    const GroupId groupId = core->getGroupPersistentId(groupNumber);
    if (!groupId.isEmpty()) {
        core->numberToGroupId[groupNumber] = groupId;
        core->groupIdToNumber[groupId] = groupNumber;
        core->startGroupReconnectTimer(groupNumber);
    }
    emit core->groupSelfJoined(groupNumber);
}

void Core::onGroupTopic(Tox* tox, uint32_t groupNumber, uint32_t peerId, const uint8_t* topic,
                        size_t length, void* vCore)
{
    std::ignore = tox;
    std::ignore = peerId;
    auto* const core = static_cast<Core*>(vCore);
    const QString newTopic = ToxString(topic, length).getQString();
    qDebug().nospace() << "Group " << groupNumber << " topic changed to " << newTopic;
    emit core->saveRequest();
    emit core->groupTopicChanged(groupNumber, newTopic);
}

void Core::onGroupJoinFail(Tox* tox, uint32_t groupNumber, Tox_Group_Join_Fail failType, void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qWarning() << "Group join failed for group" << groupNumber << "with error:" << failType;
    core->stopGroupReconnectTimer(groupNumber);
    core->groupPeerCounts.remove(groupNumber);
    const auto groupIdIt = core->numberToGroupId.find(groupNumber);
    if (groupIdIt != core->numberToGroupId.end()) {
        core->groupIdToNumber.remove(*groupIdIt);
        core->numberToGroupId.erase(groupIdIt);
    }
    emit core->groupJoinFailed(groupNumber, failType);
}

void Core::onGroupModeration(Tox* tox, uint32_t groupNumber, uint32_t sourcePeerId,
                             uint32_t targetPeerId, Tox_Group_Mod_Event modType, void* vCore)
{
    std::ignore = tox;
    std::ignore = sourcePeerId;
    std::ignore = targetPeerId;
    std::ignore = modType;
    auto* const core = static_cast<Core*>(vCore);
    qDebug() << "Group" << groupNumber << "moderation event, refreshing peer roles";
    emit core->groupPeerRolesChanged(groupNumber);
}

void Core::onGroupPassword(Tox* tox, uint32_t groupNumber, const uint8_t* password, size_t length,
                           void* vCore)
{
    std::ignore = tox;
    std::ignore = password;
    auto* const core = static_cast<Core*>(vCore);
    qDebug() << "Group" << groupNumber << "password changed, has password:" << (length != 0);
    emit core->groupPasswordChanged(groupNumber, length != 0);
}

void Core::onGroupPeerLimit(Tox* tox, uint32_t groupNumber, uint32_t peerLimit, void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qDebug() << "Group" << groupNumber << "peer limit changed to" << peerLimit;
    emit core->groupPeerLimitChanged(groupNumber, static_cast<uint16_t>(peerLimit));
}

void Core::onGroupTopicLock(Tox* tox, uint32_t groupNumber, Tox_Group_Topic_Lock topicLock,
                            void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qDebug() << "Group" << groupNumber << "topic lock changed to" << topicLock;
    emit core->groupTopicLockChanged(groupNumber, static_cast<GroupTopicLock>(topicLock));
}

void Core::onGroupVoiceState(Tox* tox, uint32_t groupNumber, Tox_Group_Voice_State voiceState,
                             void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qDebug() << "Group" << groupNumber << "voice state changed to" << voiceState;
    emit core->groupVoiceStateChanged(groupNumber, static_cast<GroupVoiceState>(voiceState));
}

void Core::onGroupPrivacyState(Tox* tox, uint32_t groupNumber, Tox_Group_Privacy_State privacyState,
                               void* vCore)
{
    std::ignore = tox;
    auto* const core = static_cast<Core*>(vCore);
    qDebug() << "Group" << groupNumber << "privacy state changed to" << privacyState;
    emit core->groupPrivacyStateChanged(groupNumber, static_cast<GroupPrivacyState>(privacyState));
}

void Core::onReadReceiptCallback(Tox* tox, uint32_t friendId, uint32_t receipt, void* core)
{
    std::ignore = tox;
    emit static_cast<Core*>(core)->receiptReceived(friendId, ReceiptNum{receipt});
}

void Core::acceptFriendRequest(const ToxPk& friendPk)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};
    Tox_Err_Friend_Add error;
    const uint32_t friendId = tox_friend_add_norequest(tox.get(), friendPk.getData(), &error);
    if (PARSE_ERR(error)) {
        emit saveRequest();
        emit friendAdded(friendId, friendPk);
    } else {
        emit failedToAddFriend(friendPk);
    }
}

/**
 * @brief Checks that sending friendship request is correct and returns error message accordingly
 * @param friendId Id of a friend which request is destined to
 * @param message Friendship request message
 * @return Returns empty string if sending request is correct, according error message otherwise
 */
QString Core::getFriendRequestErrorMessage(const ToxId& friendId, const QString& message) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (!friendId.isValid()) {
        return tr("Invalid Tox ID", "Error while sending friend request");
    }

    if (message.isEmpty()) {
        return tr("You need to write a message with your request",
                  "Error while sending friend request");
    }

    if (message.length() > static_cast<int>(tox_max_friend_request_length())) {
        return tr("Your message is too long!", "Error while sending friend request");
    }

    if (hasFriendWithPublicKey(friendId.getPublicKey())) {
        return tr("Friend is already added", "Error while sending friend request");
    }

    return QString{};
}

void Core::requestFriendship(const ToxId& friendId, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const ToxPk friendPk = friendId.getPublicKey();
    const QString errorMessage = getFriendRequestErrorMessage(friendId, message);
    if (!errorMessage.isNull()) {
        emit failedToAddFriend(friendPk, errorMessage);
        emit saveRequest();
        return;
    }

    const ToxString cMessage(message);
    Tox_Err_Friend_Add error;
    const uint32_t friendNumber =
        tox_friend_add(tox.get(), friendId.getBytes(), cMessage.data(), cMessage.size(), &error);
    if (PARSE_ERR(error)) {
        qDebug() << "Requested friendship from" << friendNumber;
        emit saveRequest();
        emit friendAdded(friendNumber, friendPk);
        emit requestSent(friendPk, message);
    } else {
        qDebug() << "Failed to send friend request";
        emit failedToAddFriend(friendPk);
    }
}

bool Core::sendMessageWithType(uint32_t friendId, const QString& message, Tox_Message_Type type,
                               ReceiptNum& receipt)
{
    const int size = message.toUtf8().size();
    auto maxSize = static_cast<int>(getMaxMessageSize());
    if (size > maxSize) {
        assert(false);
        qCritical() << "Core::sendMessageWithType called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return false;
    }

    const ToxString cMessage(message);
    Tox_Err_Friend_Send_Message error;
    receipt = ReceiptNum{tox_friend_send_message(tox.get(), friendId, type, cMessage.data(),
                                                 cMessage.size(), &error)};
    return PARSE_ERR(error);
}

bool Core::sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt)
{
    const QMutexLocker<QRecursiveMutex> ml(&coreLoopLock);
    return sendMessageWithType(friendId, message, TOX_MESSAGE_TYPE_NORMAL, receipt);
}

bool Core::sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt)
{
    const QMutexLocker<QRecursiveMutex> ml(&coreLoopLock);
    return sendMessageWithType(friendId, action, TOX_MESSAGE_TYPE_ACTION, receipt);
}

void Core::sendTyping(uint32_t friendId, bool typing)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Set_Typing error;
    tox_self_set_typing(tox.get(), friendId, typing, &error);
    if (!PARSE_ERR(error)) {
        emit failedToSetTyping(typing);
    }
}

void Core::sendConferenceMessageWithType(int conferenceId, const QString& message, Tox_Message_Type type)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const int size = message.toUtf8().size();
    auto maxSize = static_cast<int>(getMaxMessageSize());
    if (size > maxSize) {
        qCritical() << "Core::sendMessageWithType called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return;
    }

    const ToxString cMsg(message);
    Tox_Err_Conference_Send_Message error;
    tox_conference_send_message(tox.get(), conferenceId, type, cMsg.data(), cMsg.size(), &error);
    if (!PARSE_ERR(error)) {
        emit conferenceSentFailed(conferenceId);
        return;
    }
}

void Core::sendConferenceMessage(int conferenceId, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    sendConferenceMessageWithType(conferenceId, message, TOX_MESSAGE_TYPE_NORMAL);
}

void Core::sendConferenceAction(int conferenceId, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    sendConferenceMessageWithType(conferenceId, message, TOX_MESSAGE_TYPE_ACTION);
}

void Core::changeConferenceTitle(uint32_t conferenceId, const QString& title)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const ToxString cTitle(title);
    Tox_Err_Conference_Title error;
    tox_conference_set_title(tox.get(), conferenceId, cTitle.data(), cTitle.size(), &error);
    if (PARSE_ERR(error)) {
        emit saveRequest();
        emit conferenceTitleChanged(conferenceId, getUsername(), title);
    }
}

void Core::sendGroupMessageWithType(uint32_t groupNumber, const QString& message,
                                    Tox_Message_Type type)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const int size = message.toUtf8().size();
    const auto maxSize = static_cast<int>(tox_group_max_message_length());
    if (size > maxSize) {
        qCritical() << "Core::sendGroupMessageWithType called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return;
    }

    const ToxString cMsg(message);
    Tox_Err_Group_Send_Message error;
    tox_group_send_message(tox.get(), groupNumber, type, cMsg.data(), cMsg.size(), &error);
    if (!PARSE_ERR(error)) {
        emit groupSentFailed(groupNumber);
        return;
    }
}

void Core::sendGroupMessage(uint32_t groupNumber, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    sendGroupMessageWithType(groupNumber, message, TOX_MESSAGE_TYPE_NORMAL);
}

void Core::sendGroupAction(uint32_t groupNumber, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    sendGroupMessageWithType(groupNumber, message, TOX_MESSAGE_TYPE_ACTION);
}

void Core::sendGroupPrivateMessage(uint32_t groupNumber, uint32_t peerId, const QString& message,
                                   Tox_Message_Type type)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const int size = message.toUtf8().size();
    const auto maxSize = static_cast<int>(tox_group_max_message_length());
    if (size > maxSize) {
        qCritical() << "Core::sendGroupPrivateMessage called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return;
    }

    const ToxString cMsg(message);
    Tox_Err_Group_Send_Private_Message error;
    tox_group_send_private_message(tox.get(), groupNumber, peerId, type,
                                   cMsg.data(), cMsg.size(), &error);
    if (!PARSE_ERR(error)) {
        emit groupSentFailed(groupNumber);
    }
}

void Core::changeGroupTopic(uint32_t groupNumber, const QString& topic)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const ToxString cTopic(topic);
    Tox_Err_Group_Topic_Set error;
    tox_group_set_topic(tox.get(), groupNumber, cTopic.data(), cTopic.size(), &error);
    if (PARSE_ERR(error)) {
        emit saveRequest();
        emit groupTopicChanged(groupNumber, topic);
    }
}

void Core::removeFriend(uint32_t friendId)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Friend_Delete error;
    tox_friend_delete(tox.get(), friendId, &error);
    if (!PARSE_ERR(error)) {
        emit failedToRemoveFriend(friendId);
        return;
    }

    emit saveRequest();
    emit friendRemoved(friendId);
}

void Core::removeConference(int conferenceId)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Conference_Delete error;
    tox_conference_delete(tox.get(), conferenceId, &error);
    if (PARSE_ERR(error)) {
        emit saveRequest();

        /*
         * TODO(sudden6): this is probably not (thread-)safe, but can be ignored for now since
         * we don't change av at runtime.
         */

        if (av != nullptr) {
            av->leaveConferenceCall(conferenceId);
        }
    }
}

/**
 * @brief Returns our username, or an empty string on failure
 */
QString Core::getUsername() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    QString sname;
    if (!tox) {
        return sname;
    }

    const int size = tox_self_get_name_size(tox.get());
    if (size == 0) {
        return {};
    }
    std::vector<uint8_t> nameBuf(size);
    tox_self_get_name(tox.get(), nameBuf.data());
    return ToxString(nameBuf.data(), size).getQString();
}

void Core::setUsername(const QString& username)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (username == getUsername()) {
        return;
    }

    const ToxString cUsername(username);
    Tox_Err_Set_Info error;
    tox_self_set_name(tox.get(), cUsername.data(), cUsername.size(), &error);
    if (!PARSE_ERR(error)) {
        emit failedToSetUsername(username);
        return;
    }

    emit usernameSet(username);
    emit saveRequest();
}

/**
 * @brief Returns our Tox ID
 */
ToxId Core::getSelfId() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    std::vector<uint8_t> friendId(tox_address_size());
    tox_self_get_address(tox.get(), friendId.data());
    return ToxId(friendId.data(), friendId.size());
}

/**
 * @brief Gets self public key
 * @return Self PK
 */
ToxPk Core::getSelfPublicKey() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    std::vector<uint8_t> selfPk(tox_public_key_size());
    tox_self_get_public_key(tox.get(), selfPk.data());
    return ToxPk(selfPk.data());
}

QByteArray Core::getSelfDhtId() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};
    QByteArray dhtKey(tox_public_key_size(), 0x00);
    tox_self_get_dht_id(tox.get(), reinterpret_cast<uint8_t*>(dhtKey.data()));
    return dhtKey;
}

int Core::getSelfUdpPort() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};
    Tox_Err_Get_Port error;
    auto port = tox_self_get_udp_port(tox.get(), &error);
    if (!PARSE_ERR(error)) {
        return -1;
    }
    return port;
}

/**
 * @brief Returns our status message, or an empty string on failure
 */
QString Core::getStatusMessage() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    assert(tox != nullptr);

    const size_t size = tox_self_get_status_message_size(tox.get());
    if (size == 0u) {
        return {};
    }
    std::vector<uint8_t> nameBuf(size);
    tox_self_get_status_message(tox.get(), nameBuf.data());
    return ToxString(nameBuf.data(), size).getQString();
}

/**
 * @brief Returns our user status
 */
Status::Status Core::getStatus() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    return static_cast<Status::Status>(tox_self_get_status(tox.get()));
}

void Core::setStatusMessage(const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (message == getStatusMessage()) {
        return;
    }

    const ToxString cMessage(message);
    Tox_Err_Set_Info error;
    tox_self_set_status_message(tox.get(), cMessage.data(), cMessage.size(), &error);
    if (!PARSE_ERR(error)) {
        emit failedToSetStatusMessage(message);
        return;
    }

    emit saveRequest();
    emit statusMessageSet(message);
}

void Core::setStatus(Status::Status status)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_User_Status userstatus;
    switch (status) {
    case Status::Status::Online:
        userstatus = TOX_USER_STATUS_NONE;
        break;

    case Status::Status::Away:
        userstatus = TOX_USER_STATUS_AWAY;
        break;

    case Status::Status::Busy:
        userstatus = TOX_USER_STATUS_BUSY;
        break;

    default:
        return;
    }

    tox_self_set_status(tox.get(), userstatus);
    emit saveRequest();
    emit statusSet(status);
}

/**
 * @brief Returns the unencrypted tox save data
 */
QByteArray Core::getToxSaveData()
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const uint32_t fileSize = tox_get_savedata_size(tox.get());
    QByteArray data;
    data.resize(fileSize);
    tox_get_savedata(tox.get(), reinterpret_cast<uint8_t*>(data.data()));
    return data;
}

void Core::loadFriends()
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const size_t friendCount = tox_self_get_friend_list_size(tox.get());
    if (friendCount == 0) {
        return;
    }

    std::vector<uint32_t> ids(friendCount);
    tox_self_get_friend_list(tox.get(), ids.data());
    std::vector<uint8_t> friendPk(tox_public_key_size());
    for (size_t i = 0; i < friendCount; ++i) {
        Tox_Err_Friend_Get_Public_Key keyError;
        tox_friend_get_public_key(tox.get(), ids[i], friendPk.data(), &keyError);
        if (!PARSE_ERR(keyError)) {
            continue;
        }
        emit friendAdded(ids[i], ToxPk(friendPk.data()));
        emit friendUsernameChanged(ids[i], getFriendUsername(ids[i]));
        Tox_Err_Friend_Query queryError;
        const size_t statusMessageSize =
            tox_friend_get_status_message_size(tox.get(), ids[i], &queryError);
        if (PARSE_ERR(queryError) && (statusMessageSize != 0u)) {
            std::vector<uint8_t> messageData(statusMessageSize);
            tox_friend_get_status_message(tox.get(), ids[i], messageData.data(), &queryError);
            const QString friendStatusMessage =
                ToxString(messageData.data(), statusMessageSize).getQString();
            emit friendStatusMessageChanged(ids[i], friendStatusMessage);
        }
        checkLastOnline(ids[i]);
    }
}

void Core::loadConferences()
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const size_t conferenceCount = tox_conference_get_chatlist_size(tox.get());
    if (conferenceCount == 0) {
        return;
    }

    std::vector<uint32_t> conferenceNumbers(conferenceCount);
    tox_conference_get_chatlist(tox.get(), conferenceNumbers.data());

    for (size_t i = 0; i < conferenceCount; ++i) {
        Tox_Err_Conference_Title error;
        QString name;
        const auto conferenceNumber = conferenceNumbers[i];
        const size_t titleSize = tox_conference_get_title_size(tox.get(), conferenceNumber, &error);
        const ConferenceId persistentId = getConferencePersistentId(conferenceNumber);
        const QString defaultName = tr("Conference %1").arg(persistentId.toString().left(8));
        if (PARSE_ERR(error) || (titleSize == 0u)) {
            std::vector<uint8_t> nameBuf(titleSize);
            tox_conference_get_title(tox.get(), conferenceNumber, nameBuf.data(), &error);
            if (PARSE_ERR(error)) {
                name = ToxString(nameBuf.data(), titleSize).getQString();
            } else {
                name = defaultName;
            }
        } else {
            name = defaultName;
        }
        if (getConferenceAvEnabled(conferenceNumber)) {
            if (toxav_groupchat_enable_av(tox.get(), conferenceNumber,
                                          CoreAV::conferenceCallCallback, this)
                != 0) {
                qCritical() << "Failed to enable audio on loaded conference" << conferenceNumber;
            }
        }
        emit emptyConferenceCreated(conferenceNumber, persistentId, name);
    }
}

void Core::loadGroups()
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const uint32_t groupCount = tox_group_get_group_list_size(tox.get());
    QVector<uint32_t> numbers(groupCount);
    tox_group_get_group_list(tox.get(), numbers.data());

    QSet<GroupId> alreadyLoaded;
    for (const uint32_t groupNumber : numbers) {
        const GroupId groupId = getGroupPersistentId(groupNumber);
        if (groupId.isEmpty()) {
            continue;
        }
        alreadyLoaded.insert(groupId);
        if (groupIdToNumber.contains(groupId)) {
            continue;
        }
        numberToGroupId[groupNumber] = groupId;
        groupIdToNumber[groupId] = groupNumber;
        startGroupReconnectTimer(groupNumber);
        emit groupJoined(groupNumber, groupId);
    }

    const QStringList saved = settings.getSavedGroups();
    for (const QString& groupIdHex : saved) {
        if (groupIdHex.isEmpty()) {
            continue;
        }
        const QByteArray rawId = QByteArray::fromHex(groupIdHex.toLatin1());
        if (rawId.size() != TOX_GROUP_CHAT_ID_SIZE) {
            qWarning() << "loadGroups: invalid saved group id" << groupIdHex;
            continue;
        }
        const GroupId groupId(rawId);
        if (groupIdToNumber.contains(groupId) || alreadyLoaded.contains(groupId)) {
            continue;
        }

        const ToxString cSelfName(getUsername());
        Tox_Err_Group_Join error;
        const uint32_t groupNumber =
            tox_group_join(tox.get(), groupId.getData(), cSelfName.data(), cSelfName.size(), nullptr,
                           0, &error);
        if (!PARSE_ERR(error)) {
            qWarning() << "loadGroups: failed to rejoin group" << groupIdHex;
            continue;
        }

        numberToGroupId[groupNumber] = groupId;
        groupIdToNumber[groupId] = groupNumber;
        startGroupReconnectTimer(groupNumber);
        emit groupJoined(groupNumber, groupId);
    }
}

void Core::checkLastOnline(uint32_t friendId)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Friend_Get_Last_Online error;
    const uint64_t lastOnline = tox_friend_get_last_online(tox.get(), friendId, &error);
    if (PARSE_ERR(error)) {
        emit friendLastSeenChanged(friendId, QDateTime::fromSecsSinceEpoch(lastOnline));
    }
}

/**
 * @brief Returns the list of friendIds in our friend list, an empty list on error
 */
QVector<uint32_t> Core::getFriendList() const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    QVector<uint32_t> friends;
    friends.resize(tox_self_get_friend_list_size(tox.get()));
    tox_self_get_friend_list(tox.get(), friends.data());
    return friends;
}

ConferenceId Core::getConferencePersistentId(uint32_t conferenceNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    std::vector<uint8_t> idBuff(tox_conference_id_size());
    if (tox_conference_get_id(tox.get(), conferenceNumber, idBuff.data())) {
        return ConferenceId{idBuff.data()};
    }
    qCritical() << "Failed to get conference ID of conference" << conferenceNumber;
    return {};
}

/**
 * @brief Get number of peers in the conference.
 * @return The number of peers in the conference. UINT32_MAX on failure.
 */
uint32_t Core::getConferenceNumberPeers(int conferenceId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Conference_Peer_Query error;
    const uint32_t count = tox_conference_peer_count(tox.get(), conferenceId, &error);
    if (!PARSE_ERR(error)) {
        return std::numeric_limits<uint32_t>::max();
    }

    return count;
}

/**
 * @brief Get the name of a peer of a conference
 */
QString Core::getConferencePeerName(int conferenceId, int peerId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Conference_Peer_Query error;
    const size_t length = tox_conference_peer_get_name_size(tox.get(), conferenceId, peerId, &error);
    if (!PARSE_ERR(error) || (length == 0u)) {
        return QString{};
    }

    std::vector<uint8_t> nameBuf(length);
    tox_conference_peer_get_name(tox.get(), conferenceId, peerId, nameBuf.data(), &error);
    if (!PARSE_ERR(error)) {
        return QString{};
    }

    return ToxString(nameBuf.data(), length).getQString();
}

/**
 * @brief Get the public key of a peer of a conference
 */
ToxPk Core::getConferencePeerPk(int conferenceId, int peerId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    std::vector<uint8_t> friendPk(tox_public_key_size());
    Tox_Err_Conference_Peer_Query error;
    tox_conference_peer_get_public_key(tox.get(), conferenceId, peerId, friendPk.data(), &error);
    if (!PARSE_ERR(error)) {
        return ToxPk{};
    }

    return ToxPk(friendPk.data());
}

/**
 * @brief Get the names of the peers of a conference
 */
QStringList Core::getConferencePeerNames(int conferenceId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    assert(tox != nullptr);

    const uint32_t nPeers = getConferenceNumberPeers(conferenceId);
    if (nPeers == std::numeric_limits<uint32_t>::max()) {
        qWarning() << "getConferencePeerNames: Unable to get number of peers";
        return {};
    }

    QStringList names;
    for (int i = 0; i < static_cast<int>(nPeers); ++i) {
        Tox_Err_Conference_Peer_Query error;
        const size_t length = tox_conference_peer_get_name_size(tox.get(), conferenceId, i, &error);

        if (!PARSE_ERR(error) || (length == 0u)) {
            names.append(QString());
            continue;
        }

        std::vector<uint8_t> nameBuf(length);
        tox_conference_peer_get_name(tox.get(), conferenceId, i, nameBuf.data(), &error);
        if (PARSE_ERR(error)) {
            names.append(ToxString(nameBuf.data(), length).getQString());
        } else {
            names.append(QString());
        }
    }

    assert(names.size() == static_cast<int>(nPeers));

    return names;
}

/**
 * @brief Check, that conference has audio or video stream
 * @param conferenceId Id of conference to check
 * @return True for AV conferences, false for text-only conferences
 */
bool Core::getConferenceAvEnabled(int conferenceId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};
    Tox_Err_Conference_Get_Type error;
    const Tox_Conference_Type type = tox_conference_get_type(tox.get(), conferenceId, &error);
    PARSE_ERR(error);
    // would be nice to indicate to caller that we don't actually know..
    return type == TOX_CONFERENCE_TYPE_AV;
}

GroupId Core::getGroupPersistentId(uint32_t groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    QByteArray idBuff(tox_group_chat_id_size(), 0x00);
    Tox_Err_Group_State_Query error;
    if (tox_group_get_chat_id(tox.get(), groupNumber, reinterpret_cast<uint8_t*>(idBuff.data()), &error)) {
        return GroupId{reinterpret_cast<uint8_t*>(idBuff.data())};
    }
    qCritical() << "Failed to get chat id of group" << groupNumber;
    return {};
}

/**
 * @brief Get the number of peers in a group.
 * @return The number of peers in the group. UINT32_MAX on failure.
 */
uint32_t Core::getGroupNumberPeers(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    // NGC has no public peer enumeration API. The peer count is tracked
    // client-side by the Group model.
    std::ignore = groupNumber;
    return std::numeric_limits<uint32_t>::max();
}

/**
 * @brief Get the self peer id of a group.
 * @return The self peer id on success, UINT32_MAX on failure.
 */
uint32_t Core::getGroupSelfPeerId(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Self_Query error;
    const uint32_t peerId = tox_group_self_get_peer_id(tox.get(), groupNumber, &error);
    if (!PARSE_ERR(error)) {
        return std::numeric_limits<uint32_t>::max();
    }

    return peerId;
}

/**
 * @brief Get the name of a peer of a group
 */
QString Core::getGroupPeerName(int groupNumber, int peerId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Peer_Query error;
    const size_t length = tox_group_peer_get_name_size(tox.get(), groupNumber, peerId, &error);
    if (!PARSE_ERR(error) || (length == 0u)) {
        return QString{};
    }

    QByteArray nameBuf(length, 0x00);
    tox_group_peer_get_name(tox.get(), groupNumber, peerId, reinterpret_cast<uint8_t*>(nameBuf.data()), &error);
    if (!PARSE_ERR(error)) {
        return QString{};
    }

    return ToxString(reinterpret_cast<uint8_t*>(nameBuf.data()), length).getQString();
}

/**
 * @brief Get the public key of a peer of a group
 */
ToxPk Core::getGroupPeerPk(int groupNumber, int peerId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Self_Query selfError;
    const uint32_t selfPeerId = tox_group_self_get_peer_id(tox.get(), groupNumber, &selfError);
    if (PARSE_ERR(selfError) && selfPeerId == static_cast<uint32_t>(peerId)) {
        return getGroupSelfPk(groupNumber);
    }

    QByteArray peerPk(tox_public_key_size(), 0x00);
    Tox_Err_Group_Peer_Query error;
    tox_group_peer_get_public_key(tox.get(), groupNumber, peerId, reinterpret_cast<uint8_t*>(peerPk.data()), &error);
    if (!PARSE_ERR(error)) {
        return ToxPk{};
    }

    return ToxPk(reinterpret_cast<uint8_t*>(peerPk.data()));
}

/**
 * @brief Get the public key identifying us in a group.
 *
 * Unlike friend chats, our identity within a group is a dedicated per-group
 * peer public key generated by toxcore, not our profile's public key.
 */
ToxPk Core::getGroupSelfPk(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    QByteArray selfPk(tox_public_key_size(), 0x00);
    Tox_Err_Group_Self_Query error;
    tox_group_self_get_public_key(tox.get(), groupNumber,
                                  reinterpret_cast<uint8_t*>(selfPk.data()), &error);
    if (!PARSE_ERR(error)) {
        return ToxPk{};
    }

    return ToxPk(reinterpret_cast<uint8_t*>(selfPk.data()));
}

/**
 * @brief Get the role of a peer in a group
 */
GroupRole Core::getGroupPeerRole(int groupNumber, int peerId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Peer_Query error;
    const Tox_Group_Role role = tox_group_peer_get_role(tox.get(), groupNumber, peerId, &error);
    if (!PARSE_ERR(error)) {
        return GroupRole::Unknown;
    }

    return static_cast<GroupRole>(role);
}

bool Core::setGroupPeerRole(int groupNumber, int peerId, GroupRole role)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const auto toxRole = static_cast<Tox_Group_Role>(role);
    Tox_Err_Group_Set_Role error;
    const bool success = tox_group_set_role(tox.get(), groupNumber, peerId, toxRole, &error);
    if (!success) {
        qWarning() << "Failed to set role" << static_cast<int>(role) << "for peer" << peerId
                   << "in group" << groupNumber << ":" << tox_err_group_set_role_to_string(error);
        return false;
    }

    emit groupPeerRolesChanged(groupNumber);
    return true;
}

bool Core::kickGroupPeer(int groupNumber, int peerId)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Kick_Peer error;
    const bool success = tox_group_kick_peer(tox.get(), groupNumber, peerId, &error);
    if (!success) {
        qWarning() << "Failed to kick peer" << peerId << "from group" << groupNumber << ":"
                   << tox_err_group_kick_peer_to_string(error);
        return false;
    }

    return true;
}

bool Core::setGroupPassword(int groupNumber, const QByteArray& password)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (password.size() > TOX_GROUP_MAX_PASSWORD_SIZE) {
        qWarning() << "Failed to set password for group" << groupNumber << ": password too long";
        return false;
    }

    const auto* const passwordData =
        password.isEmpty() ? nullptr : reinterpret_cast<const uint8_t*>(password.constData());
    Tox_Err_Group_Set_Password error;
    const bool success = tox_group_set_password(tox.get(), groupNumber, passwordData, password.size(),
                                                &error);
    if (!success) {
        qWarning() << "Failed to set password for group" << groupNumber << ":"
                   << tox_err_group_set_password_to_string(error);
        return false;
    }

    emit groupPasswordChanged(groupNumber, !password.isEmpty());
    return true;
}

bool Core::setGroupPeerLimit(int groupNumber, uint16_t peerLimit)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Set_Peer_Limit error;
    const bool success = tox_group_set_peer_limit(tox.get(), groupNumber, peerLimit, &error);
    if (!success) {
        qWarning() << "Failed to set peer limit" << peerLimit << "for group" << groupNumber << ":"
                   << tox_err_group_set_peer_limit_to_string(error);
        return false;
    }

    emit groupPeerLimitChanged(groupNumber, peerLimit);
    return true;
}

bool Core::setGroupTopicLock(int groupNumber, GroupTopicLock topicLock)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const auto toxTopicLock = static_cast<Tox_Group_Topic_Lock>(topicLock);
    Tox_Err_Group_Set_Topic_Lock error;
    const bool success = tox_group_set_topic_lock(tox.get(), groupNumber, toxTopicLock, &error);
    if (!success) {
        qWarning() << "Failed to set topic lock" << static_cast<int>(topicLock) << "for group"
                   << groupNumber << ":" << tox_err_group_set_topic_lock_to_string(error);
        return false;
    }

    emit groupTopicLockChanged(groupNumber, topicLock);
    return true;
}

bool Core::setGroupVoiceState(int groupNumber, GroupVoiceState voiceState)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const auto toxVoiceState = static_cast<Tox_Group_Voice_State>(voiceState);
    Tox_Err_Group_Set_Voice_State error;
    const bool success = tox_group_set_voice_state(tox.get(), groupNumber, toxVoiceState, &error);
    if (!success) {
        qWarning() << "Failed to set voice state" << static_cast<int>(voiceState) << "for group"
                   << groupNumber << ":" << tox_err_group_set_voice_state_to_string(error);
        return false;
    }

    emit groupVoiceStateChanged(groupNumber, voiceState);
    return true;
}

bool Core::setGroupPrivacyState(int groupNumber, GroupPrivacyState privacyState)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const auto toxPrivacyState = static_cast<Tox_Group_Privacy_State>(privacyState);
    Tox_Err_Group_Set_Privacy_State error;
    const bool success = tox_group_set_privacy_state(tox.get(), groupNumber, toxPrivacyState, &error);
    if (!success) {
        qWarning() << "Failed to set privacy state" << static_cast<int>(privacyState) << "for group"
                   << groupNumber << ":" << tox_err_group_set_privacy_state_to_string(error);
        return false;
    }

    emit groupPrivacyStateChanged(groupNumber, privacyState);
    return true;
}

bool Core::getGroupHasPassword(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_State_Query error;
    const size_t passwordSize = tox_group_get_password_size(tox.get(), groupNumber, &error);
    if (PARSE_ERR(error)) {
        return passwordSize != 0;
    }

    return false;
}

uint16_t Core::getGroupPeerLimit(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_State_Query error;
    const uint16_t peerLimit = tox_group_get_peer_limit(tox.get(), groupNumber, &error);
    if (PARSE_ERR(error)) {
        return peerLimit;
    }

    return 0;
}

GroupTopicLock Core::getGroupTopicLock(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_State_Query error;
    const auto topicLock = tox_group_get_topic_lock(tox.get(), groupNumber, &error);
    if (PARSE_ERR(error)) {
        return static_cast<GroupTopicLock>(topicLock);
    }

    return GroupTopicLock::Unknown;
}

GroupVoiceState Core::getGroupVoiceState(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_State_Query error;
    const auto voiceState = tox_group_get_voice_state(tox.get(), groupNumber, &error);
    if (PARSE_ERR(error)) {
        return static_cast<GroupVoiceState>(voiceState);
    }

    return GroupVoiceState::Unknown;
}

GroupPrivacyState Core::getGroupPrivacyState(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_State_Query error;
    const auto privacyState = tox_group_get_privacy_state(tox.get(), groupNumber, &error);
    if (PARSE_ERR(error)) {
        return static_cast<GroupPrivacyState>(privacyState);
    }

    return GroupPrivacyState::Unknown;
}

/**
 * @brief Get the name of a group
 */
QString Core::getGroupTitle(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_State_Query error;
    const size_t length = tox_group_get_name_size(tox.get(), groupNumber, &error);
    if (!PARSE_ERR(error) || (length == 0u)) {
        return QString{};
    }

    QByteArray nameBuf(length, 0x00);
    tox_group_get_name(tox.get(), groupNumber, reinterpret_cast<uint8_t*>(nameBuf.data()), &error);
    if (!PARSE_ERR(error)) {
        return QString{};
    }

    return ToxString(reinterpret_cast<uint8_t*>(nameBuf.data()), length).getQString();
}

/**
 * @brief Get the topic of a group
 */
QString Core::getGroupTopic(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_State_Query error;
    const size_t length = tox_group_get_topic_size(tox.get(), groupNumber, &error);
    if (!PARSE_ERR(error) || (length == 0u)) {
        return QString{};
    }

    QByteArray topicBuf(length, 0x00);
    tox_group_get_topic(tox.get(), groupNumber, reinterpret_cast<uint8_t*>(topicBuf.data()), &error);
    if (!PARSE_ERR(error)) {
        return QString{};
    }

    return ToxString(reinterpret_cast<uint8_t*>(topicBuf.data()), length).getQString();
}

/**
 * @brief Get the self name in a group
 */
QString Core::getGroupSelfName(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Self_Query error;
    const size_t length = tox_group_self_get_name_size(tox.get(), groupNumber, &error);
    if (!PARSE_ERR(error) || (length == 0u)) {
        return QString{};
    }

    QByteArray nameBuf(length, 0x00);
    tox_group_self_get_name(tox.get(), groupNumber, reinterpret_cast<uint8_t*>(nameBuf.data()), &error);
    if (!PARSE_ERR(error)) {
        return QString{};
    }

    return ToxString(reinterpret_cast<uint8_t*>(nameBuf.data()), length).getQString();
}

/**
 * @brief Set the self name in a group
 */
bool Core::setGroupSelfName(int groupNumber, const QString& name)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const ToxString toxName(name);
    Tox_Err_Group_Self_Name_Set error;
    const bool success = tox_group_self_set_name(tox.get(), groupNumber,
                                                  toxName.data(),
                                                  toxName.size(), &error);
    if (!success) {
        qWarning() << "Failed to set group self name for group" << groupNumber << ":"
                   << static_cast<int>(error);
        return false;
    }

    return true;
}

/**
 * @brief Get the self status in a group
 */
Status::Status Core::getGroupSelfStatus(int groupNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Self_Query error;
    const Tox_User_Status status = tox_group_self_get_status(tox.get(), groupNumber, &error);
    if (!PARSE_ERR(error)) {
        return Status::Status::Offline;
    }

    return static_cast<Status::Status>(status);
}

/**
 * @brief Set the self status in a group
 */
bool Core::setGroupSelfStatus(int groupNumber, Status::Status status)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Self_Status_Set error;
    const bool success = tox_group_self_set_status(tox.get(), groupNumber,
                                                    static_cast<Tox_User_Status>(status), &error);
    if (!success) {
        qWarning() << "Failed to set group self status for group" << groupNumber << ":"
                   << static_cast<int>(error);
        return false;
    }

    return true;
}

/**
 * @brief Get the status of a peer in a group
 */
Status::Status Core::getGroupPeerStatus(int groupNumber, int peerId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Peer_Query error;
    const Tox_User_Status status = tox_group_peer_get_status(tox.get(), groupNumber, peerId, &error);
    if (!PARSE_ERR(error)) {
        return Status::Status::Offline;
    }

    return static_cast<Status::Status>(status);
}

/**
 * @brief Accept a conference invite.
 * @param inviteInfo Object which contains info about conference invitation
 *
 * @return Conference number on success, UINT32_MAX on failure.
 */
uint32_t Core::joinConference(const ConferenceInvite& inviteInfo)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const uint32_t friendId = inviteInfo.getFriendId();
    const uint8_t confType = inviteInfo.getType();
    const QByteArray invite = inviteInfo.getInvite();
    const auto* const cookie = reinterpret_cast<const uint8_t*>(invite.data());
    const size_t cookieLength = invite.length();
    uint32_t conferenceNum{std::numeric_limits<uint32_t>::max()};
    switch (confType) {
    case TOX_CONFERENCE_TYPE_TEXT: {
        qDebug() << "Trying to accept invite for text conference sent by friend" << friendId;
        Tox_Err_Conference_Join error;
        conferenceNum = tox_conference_join(tox.get(), friendId, cookie, cookieLength, &error);
        if (!PARSE_ERR(error)) {
            conferenceNum = std::numeric_limits<uint32_t>::max();
        }
        break;
    }
    case TOX_CONFERENCE_TYPE_AV: {
        qDebug() << "Trying to join AV conference invite sent by friend" << friendId;
        conferenceNum = toxav_join_av_groupchat(tox.get(), friendId, cookie, cookieLength,
                                                CoreAV::conferenceCallCallback, this);
        break;
    }
    default:
        qWarning() << "joinConference: Unknown conference type" << confType;
    }
    if (conferenceNum != std::numeric_limits<uint32_t>::max()) {
        emit saveRequest();
        emit conferenceJoined(conferenceNum, getConferencePersistentId(conferenceNum));
    }
    return conferenceNum;
}

void Core::conferenceInviteFriend(uint32_t friendId, int conferenceId)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Conference_Invite error;
    tox_conference_invite(tox.get(), friendId, conferenceId, &error);
    PARSE_ERR(error);
}

int Core::createConference(uint8_t type)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (type == TOX_CONFERENCE_TYPE_TEXT) {
        Tox_Err_Conference_New error;
        const uint32_t conferenceId = tox_conference_new(tox.get(), &error);
        if (PARSE_ERR(error)) {
            emit saveRequest();
            emit emptyConferenceCreated(conferenceId, getConferencePersistentId(conferenceId));
            return conferenceId;
        }
        return std::numeric_limits<uint32_t>::max();
    }
    if (type == TOX_CONFERENCE_TYPE_AV) {
        // unlike tox_conference_new, toxav_add_av_groupchat does not have an error enum, so -1
        // conference number is our only indication of an error
        const int conferenceId =
            toxav_add_av_groupchat(tox.get(), CoreAV::conferenceCallCallback, this);
        if (conferenceId != -1) {
            emit saveRequest();
            emit emptyConferenceCreated(conferenceId, getConferencePersistentId(conferenceId));
        } else {
            qCritical() << "Unknown error creating AV conference";
        }
        return conferenceId;
    }
    qWarning() << "createConference: Unknown type" << type;
    return -1;
}

void Core::groupInviteFriend(uint32_t friendId, int groupNumber)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Invite_Friend error;
    tox_group_invite_friend(tox.get(), groupNumber, friendId, &error);
    if (!PARSE_ERR(error)) {
        qWarning() << "Failed to invite friend" << friendId << "to group" << groupNumber;
    }
}

int Core::createGroup(const QString& groupName)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const ToxString cName(groupName);
    const ToxString cSelfName(getUsername());
    Tox_Err_Group_New error;
    const uint32_t groupNumber =
        tox_group_new(tox.get(), TOX_GROUP_PRIVACY_STATE_PUBLIC, cName.data(), cName.size(),
                      cSelfName.data(), cSelfName.size(), &error);
    if (!PARSE_ERR(error)) {
        qCritical() << "Failed to create group";
        return -1;
    }

    const GroupId groupId = getGroupPersistentId(groupNumber);
    numberToGroupId[groupNumber] = groupId;
    groupIdToNumber[groupId] = groupNumber;
    startGroupReconnectTimer(groupNumber);

    emit saveRequest();
    emit emptyGroupCreated(groupNumber, groupId, groupName);
    return groupNumber;
}

/**
 * @brief Accept a group invite.
 * @param inviteInfo Object which contains info about group invitation
 *
 * @return Group number on success, UINT32_MAX on failure.
 */
uint32_t Core::joinGroup(const GroupInvite& inviteInfo)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    const uint32_t friendId = inviteInfo.getFriendId();
    const QByteArray invite = inviteInfo.getInviteData();
    const auto* const inviteData = reinterpret_cast<const uint8_t*>(invite.constData());
    const size_t inviteLength = invite.size();

    if (inviteLength < TOX_GROUP_CHAT_ID_SIZE) {
        qWarning() << "joinGroup: invite data too short";
        return std::numeric_limits<uint32_t>::max();
    }

    const GroupId groupId(inviteData);
    if (groupIdToNumber.contains(groupId)) {
        qDebug() << "joinGroup: already in group" << groupId.toString();
        return groupIdToNumber[groupId];
    }

    const ToxString cSelfName(getUsername());

    qDebug() << "Trying to accept invite for group sent by friend" << friendId;
    Tox_Err_Group_Invite_Accept error;
    const uint32_t groupNumber =
        tox_group_invite_accept(tox.get(), friendId, inviteData, inviteLength, cSelfName.data(),
                                cSelfName.size(), nullptr, 0, &error);
    if (!PARSE_ERR(error)) {
        qWarning() << "Failed to accept group invite from friend" << friendId;
        return std::numeric_limits<uint32_t>::max();
    }

    numberToGroupId[groupNumber] = groupId;
    groupIdToNumber[groupId] = groupNumber;
    startGroupReconnectTimer(groupNumber);

    emit saveRequest();
    emit groupJoined(groupNumber, groupId);
    return groupNumber;
}

/**
 * @brief Join an NGC group by its chat id.
 * @param groupId Chat ID of the group to join.
 * @return Group number on success, -1 on failure.
 */
int Core::joinGroup(const GroupId& groupId)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (groupId.isEmpty()) {
        qWarning() << "joinGroup: empty group id";
        return -1;
    }

    if (groupIdToNumber.contains(groupId)) {
        return groupIdToNumber[groupId];
    }

    const ToxString cSelfName(getUsername());
    Tox_Err_Group_Join error;
    const uint32_t groupNumber =
        tox_group_join(tox.get(), groupId.getData(), cSelfName.data(), cSelfName.size(), nullptr, 0,
                       &error);
    if (!PARSE_ERR(error)) {
        qWarning() << "Failed to join group" << groupId.toString();
        return -1;
    }

    numberToGroupId[groupNumber] = groupId;
    groupIdToNumber[groupId] = groupNumber;
    startGroupReconnectTimer(groupNumber);

    emit saveRequest();
    emit groupJoined(groupNumber, groupId);
    return groupNumber;
}

void Core::quitGroup(int groupNumber)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Leave error;
    tox_group_leave(tox.get(), groupNumber, nullptr, 0, &error);
    if (PARSE_ERR(error)) {
        const auto groupIdIt = numberToGroupId.find(groupNumber);
        if (groupIdIt != numberToGroupId.end()) {
            groupIdToNumber.remove(*groupIdIt);
            numberToGroupId.erase(groupIdIt);
        }
        stopGroupReconnectTimer(groupNumber);
        groupPeerCounts.remove(groupNumber);
        emit saveRequest();
        emit groupSelfDisconnected(groupNumber);
    }
}

bool Core::reconnectGroup(uint32_t groupNumber)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Group_Reconnect error;
    const bool success = tox_group_reconnect(tox.get(), groupNumber, &error);
    if (!success) {
        qWarning() << "Failed to reconnect group" << groupNumber << ":"
                   << tox_err_group_reconnect_to_string(error);
    }
    return success;
}

void Core::retryGroupReconnect(uint32_t groupNumber)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (!numberToGroupId.contains(groupNumber)) {
        stopGroupReconnectTimer(groupNumber);
        return;
    }

    // group has other members, toxcore keeps it connected on its own
    if (groupPeerCounts.value(groupNumber, 0) > 0) {
        stopGroupReconnectTimer(groupNumber);
        return;
    }

    reconnectGroup(groupNumber);
}

void Core::startGroupReconnectTimer(uint32_t groupNumber)
{
    QMetaObject::invokeMethod(this, [this, groupNumber] {
        if (groupReconnectTimers.contains(groupNumber)) {
            return;
        }

        auto* timer = new QTimer(this);
        timer->setInterval(GROUP_RECONNECT_INTERVAL_MS);
        connect(timer, &QTimer::timeout, this,
                [this, groupNumber] { retryGroupReconnect(groupNumber); });
        groupReconnectTimers[groupNumber] = timer;
        timer->start();
    });
}

void Core::stopGroupReconnectTimer(uint32_t groupNumber)
{
    QMetaObject::invokeMethod(this, [this, groupNumber] {
        auto it = groupReconnectTimers.find(groupNumber);
        if (it != groupReconnectTimers.end()) {
            it.value()->deleteLater();
            groupReconnectTimers.erase(it);
        }
    });
}

/**
 * @brief Checks if a friend is online. Unknown friends are considered offline.
 */
bool Core::isFriendOnline(uint32_t friendId) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Friend_Query error;
    const Tox_Connection connection = tox_friend_get_connection_status(tox.get(), friendId, &error);
    PARSE_ERR(error);
    return connection != TOX_CONNECTION_NONE;
}

/**
 * @brief Checks if we have a friend by public key
 */
bool Core::hasFriendWithPublicKey(const ToxPk& publicKey) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (publicKey.isEmpty()) {
        return false;
    }

    Tox_Err_Friend_By_Public_Key error;
    (void)tox_friend_by_public_key(tox.get(), publicKey.getData(), &error);
    return PARSE_ERR(error);
}

/**
 * @brief Get the public key part of the ToxID only
 */
ToxPk Core::getFriendPublicKey(uint32_t friendNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    std::vector<uint8_t> rawid(tox_public_key_size());
    Tox_Err_Friend_Get_Public_Key error;
    tox_friend_get_public_key(tox.get(), friendNumber, rawid.data(), &error);
    if (!PARSE_ERR(error)) {
        qWarning() << "getFriendPublicKey: Getting public key failed";
        return {};
    }

    return ToxPk(rawid.data());
}

/**
 * @brief Get the username of a friend
 */
QString Core::getFriendUsername(uint32_t friendNumber) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Friend_Query error;
    const size_t nameSize = tox_friend_get_name_size(tox.get(), friendNumber, &error);
    if (!PARSE_ERR(error) || (nameSize == 0u)) {
        return {};
    }

    std::vector<uint8_t> nameBuf(nameSize);
    tox_friend_get_name(tox.get(), friendNumber, nameBuf.data(), &error);
    if (!PARSE_ERR(error)) {
        return {};
    }
    return ToxString(nameBuf.data(), nameSize).getQString();
}

uint64_t Core::getMaxMessageSize()
{
    /*
     * TODO: Remove this hack; the reported max message length we receive from c-toxcore
     * as of 08-02-2019 is inaccurate, causing us to generate too large messages when splitting
     * them up.
     *
     * The inconsistency lies in c-toxcore group.c:2480 using MAX_GROUP_MESSAGE_DATA_LEN to verify
     * message size is within limit, but tox_max_message_length giving a different size limit to us.
     *
     * (uint32_t tox_max_message_length(void); declared in tox.h, unable to see explicit definition)
     */
    return tox_max_message_length() - 50;
}

QString Core::getPeerName(const ToxPk& id) const
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    Tox_Err_Friend_By_Public_Key keyError;
    const uint32_t friendId = tox_friend_by_public_key(tox.get(), id.getData(), &keyError);
    if (!PARSE_ERR(keyError)) {
        qWarning() << "getPeerName: No such peer";
        return {};
    }

    Tox_Err_Friend_Query queryError;
    const size_t nameSize = tox_friend_get_name_size(tox.get(), friendId, &queryError);
    if (!PARSE_ERR(queryError) || (nameSize == 0u)) {
        return {};
    }

    std::vector<uint8_t> nameBuf(nameSize);
    tox_friend_get_name(tox.get(), friendId, nameBuf.data(), &queryError);
    if (!PARSE_ERR(queryError)) {
        qWarning() << "getPeerName: Can't get name of friend" << friendId;
        return {};
    }

    return ToxString(nameBuf.data(), nameSize).getQString();
}

/**
 * @brief Sets the NoSpam value to prevent friend request spam
 * @param nospam an arbitrary which becomes part of the Tox ID
 */
void Core::setNospam(uint32_t nospam)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    tox_self_set_nospam(tox.get(), nospam);
    emit idSet(getSelfId());
}
