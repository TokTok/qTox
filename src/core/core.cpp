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
#include "src/model/ibootstraplistgenerator.h"
#include "src/model/status.h"
#include "src/persistence/profile.h"
#include "util/toxcoreerrorparser.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QRegularExpression>
#include <QString>
#include <QStringBuilder>
#include <QTimer>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>
#include <random>
#include <tox/tox.h>
#include <tox/tox_events.h>

const QString Core::TOX_EXT = ".tox";

#define ASSERT_CORE_THREAD assert(QThread::currentThread() == coreThread.get())

namespace {

QList<DhtServer> shuffleBootstrapNodes(QList<DhtServer> bootstrapNodes)
{
    std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::shuffle(bootstrapNodes.begin(), bootstrapNodes.end(), rng);
    return bootstrapNodes;
}

struct Tox_Events_Deleter
{
    [[maybe_unused]]
    void operator()(Tox_Events* events)
    {
        tox_events_free(events);
    }
};

Tox_Message_Type toToxEnum(Core::MessageType type)
{
    switch (type) {
    case Core::MessageType::NORMAL:
        return TOX_MESSAGE_TYPE_NORMAL;
    case Core::MessageType::ACTION:
        return TOX_MESSAGE_TYPE_ACTION;
    }
    qFatal("Unknown message type: %d", static_cast<int>(type));
}

ConferenceType fromToxEnum(Tox_Conference_Type type)
{
    switch (type) {
    case TOX_CONFERENCE_TYPE_TEXT:
        return ConferenceType::TEXT;
    case TOX_CONFERENCE_TYPE_AV:
        return ConferenceType::AV;
    }
    return ConferenceType::UNKNOWN;
}

} // namespace

void Core::Tox_Deleter::operator()(Tox* tox_) const
{
    tox_kill(tox_);
}

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
    assert(ToxPk::size == tox_public_key_size());
    assert(ConferenceId::size == tox_conference_id_size());
    assert(ToxId::size == tox_address_size());
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

    tox_events_init(core->tox.get());

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

void Core::dispatchEvent(const Tox_Event* event)
{
    switch (tox_event_get_type(event)) {
    case TOX_EVENT_FRIEND_REQUEST:
        return onFriendRequest(tox_event_get_friend_request(event), this);
    case TOX_EVENT_FRIEND_CONNECTION_STATUS:
        return onFriendConnectionStatus(tox_event_get_friend_connection_status(event), this);
    case TOX_EVENT_FRIEND_LOSSY_PACKET:
        break;
    case TOX_EVENT_FRIEND_LOSSLESS_PACKET:
        break;
    case TOX_EVENT_FRIEND_NAME:
        return onFriendName(tox_event_get_friend_name(event), this);
    case TOX_EVENT_FRIEND_STATUS:
        return onFriendStatus(tox_event_get_friend_status(event), this);
    case TOX_EVENT_FRIEND_STATUS_MESSAGE:
        return onFriendStatusMessage(tox_event_get_friend_status_message(event), this);
    case TOX_EVENT_FRIEND_MESSAGE:
        return onFriendMessage(tox_event_get_friend_message(event), this);
    case TOX_EVENT_FRIEND_READ_RECEIPT:
        return onFriendReadReceipt(tox_event_get_friend_read_receipt(event), this);
    case TOX_EVENT_FRIEND_TYPING:
        return onFriendTyping(tox_event_get_friend_typing(event), this);
    case TOX_EVENT_FILE_CHUNK_REQUEST:
        return CoreFile::onFileChunkRequest(tox_event_get_file_chunk_request(event), this);
    case TOX_EVENT_FILE_RECV:
        return CoreFile::onFileRecv(tox_event_get_file_recv(event), this);
    case TOX_EVENT_FILE_RECV_CHUNK:
        return CoreFile::onFileRecvChunk(tox_event_get_file_recv_chunk(event), this);
    case TOX_EVENT_FILE_RECV_CONTROL:
        return CoreFile::onFileRecvControl(tox_event_get_file_recv_control(event), this);
    case TOX_EVENT_CONFERENCE_INVITE:
        return onConferenceInvite(tox_event_get_conference_invite(event), this);
    case TOX_EVENT_CONFERENCE_CONNECTED:
        break;
    case TOX_EVENT_CONFERENCE_PEER_LIST_CHANGED:
        return onConferencePeerListChanged(tox_event_get_conference_peer_list_changed(event), this);
    case TOX_EVENT_CONFERENCE_PEER_NAME:
        return onConferencePeerName(tox_event_get_conference_peer_name(event), this);
    case TOX_EVENT_CONFERENCE_TITLE:
        return onConferenceTitle(tox_event_get_conference_title(event), this);
    case TOX_EVENT_CONFERENCE_MESSAGE:
        return onConferenceMessage(tox_event_get_conference_message(event), this);
    case TOX_EVENT_GROUP_PEER_NAME:
        break;
    case TOX_EVENT_GROUP_PEER_STATUS:
        break;
    case TOX_EVENT_GROUP_TOPIC:
        break;
    case TOX_EVENT_GROUP_PRIVACY_STATE:
        break;
    case TOX_EVENT_GROUP_VOICE_STATE:
        break;
    case TOX_EVENT_GROUP_TOPIC_LOCK:
        break;
    case TOX_EVENT_GROUP_PEER_LIMIT:
        break;
    case TOX_EVENT_GROUP_PASSWORD:
        break;
    case TOX_EVENT_GROUP_MESSAGE:
        break;
    case TOX_EVENT_GROUP_PRIVATE_MESSAGE:
        break;
    case TOX_EVENT_GROUP_CUSTOM_PACKET:
        break;
    case TOX_EVENT_GROUP_CUSTOM_PRIVATE_PACKET:
        break;
    case TOX_EVENT_GROUP_INVITE:
        break;
    case TOX_EVENT_GROUP_PEER_JOIN:
        break;
    case TOX_EVENT_GROUP_PEER_EXIT:
        break;
    case TOX_EVENT_GROUP_SELF_JOIN:
        break;
    case TOX_EVENT_GROUP_JOIN_FAIL:
        break;
    case TOX_EVENT_GROUP_MODERATION:
        break;
    case TOX_EVENT_DHT_GET_NODES_RESPONSE:
        break;
    default:
        qWarning() << "Unknown event type" << tox_event_type_to_string(tox_event_get_type(event));
        break;
    }
}

/**
 * @brief Processes toxcore events and ensure we stay connected, called by its own timer
 */
void Core::process()
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    ASSERT_CORE_THREAD;

    Tox_Err_Events_Iterate err;
    std::unique_ptr<Tox_Events, Tox_Events_Deleter> events{tox_events_iterate(tox.get(), true, &err)};
    if (err != TOX_ERR_EVENTS_ITERATE_OK) {
        PARSE_ERR(err);
    } else if (events != nullptr) {
        for (uint32_t i = 0; i < tox_events_get_size(events.get()); ++i) {
            const Tox_Event* event = tox_events_get(events.get(), i);
            dispatchEvent(event);
        }
    }

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

void Core::onFriendRequest(const Tox_Event_Friend_Request* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const ToxPk friendPk(tox_event_friend_request_get_public_key(event));
    const QString requestMessage = ToxString(tox_event_friend_request_get_message(event),
                                             tox_event_friend_request_get_message_length(event))
                                       .getQString();
    emit core->friendRequestReceived(friendPk, requestMessage);
}

void Core::onFriendMessage(const Tox_Event_Friend_Message* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_friend_message_get_friend_number(event);
    const auto type = tox_event_friend_message_get_type(event);
    const auto* const message = tox_event_friend_message_get_message(event);
    const auto length = tox_event_friend_message_get_message_length(event);
    const bool isAction = (type == TOX_MESSAGE_TYPE_ACTION);
    const QString msg = ToxString(message, length).getQString();
    emit core->friendMessageReceived(friendId, msg, isAction);
}

void Core::onFriendName(const Tox_Event_Friend_Name* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_friend_name_get_friend_number(event);
    const auto* const name = tox_event_friend_name_get_name(event);
    const auto length = tox_event_friend_name_get_name_length(event);
    const QString newName = ToxString(name, length).getQString();
    // no saveRequest, this callback is called on every connection, not just on name change
    emit core->friendUsernameChanged(friendId, newName);
}

void Core::onFriendTyping(const Tox_Event_Friend_Typing* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_friend_typing_get_friend_number(event);
    const auto isTyping = tox_event_friend_typing_get_typing(event);
    emit core->friendTypingChanged(friendId, isTyping);
}

void Core::onFriendStatusMessage(const Tox_Event_Friend_Status_Message* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_friend_status_message_get_friend_number(event);
    const auto* const message = tox_event_friend_status_message_get_message(event);
    const auto length = tox_event_friend_status_message_get_message_length(event);
    const QString msg = ToxString(message, length).getQString();
    // no saveRequest, this callback is called on every connection, not just on name change
    emit core->friendStatusMessageChanged(friendId, msg);
}

void Core::onFriendStatus(const Tox_Event_Friend_Status* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_friend_status_get_friend_number(event);
    const auto userstatus = tox_event_friend_status_get_status(event);
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
    emit core->friendStatusChanged(friendId, status);
}

void Core::onFriendConnectionStatus(const Tox_Event_Friend_Connection_Status* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_friend_connection_status_get_friend_number(event);
    const auto status = tox_event_friend_connection_status_get_connection_status(event);
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

void Core::onConferenceInvite(const Tox_Event_Conference_Invite* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_conference_invite_get_friend_number(event);
    const auto type = tox_event_conference_invite_get_type(event);
    const auto* const cookie = tox_event_conference_invite_get_cookie(event);
    const auto length = tox_event_conference_invite_get_cookie_length(event);
    const QByteArray data(reinterpret_cast<const char*>(cookie), length);
    const ConferenceInvite inviteInfo(friendId, fromToxEnum(type), data);
    switch (fromToxEnum(type)) {
    case ConferenceType::TEXT:
        qDebug() << "Text conference invite by" << friendId;
        emit core->conferenceInviteReceived(inviteInfo);
        break;

    case ConferenceType::AV:
        qDebug() << "AV conference invite by" << friendId;
        emit core->conferenceInviteReceived(inviteInfo);
        break;

    case ConferenceType::UNKNOWN:
        qCritical() << "Conference invite with unknown type" << type;
        break;
    }
}

void Core::onConferenceMessage(const Tox_Event_Conference_Message* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto conferenceId = tox_event_conference_message_get_conference_number(event);
    const auto peerId = tox_event_conference_message_get_peer_number(event);
    const auto type = tox_event_conference_message_get_type(event);
    const auto* const message = tox_event_conference_message_get_message(event);
    const auto length = tox_event_conference_message_get_message_length(event);
    bool isAction = type == TOX_MESSAGE_TYPE_ACTION;
    QString msg = ToxString(message, length).getQString();
    emit core->conferenceMessageReceived(conferenceId, peerId, msg, isAction);
}

void Core::onConferencePeerListChanged(const Tox_Event_Conference_Peer_List_Changed* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto conferenceId = tox_event_conference_peer_list_changed_get_conference_number(event);
    qDebug("Conference %u peerlist changed", conferenceId);
    emit core->conferencePeerlistChanged(conferenceId);
}

void Core::onConferencePeerName(const Tox_Event_Conference_Peer_Name* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto conferenceId = tox_event_conference_peer_name_get_conference_number(event);
    const auto peerId = tox_event_conference_peer_name_get_peer_number(event);
    const auto* const name = tox_event_conference_peer_name_get_name(event);
    const auto length = tox_event_conference_peer_name_get_name_length(event);
    const auto newName = ToxString(name, length).getQString();
    qDebug().nospace() << "Conference " << conferenceId << ", peer " << peerId << ", name " << newName;
    auto peerPk = core->getConferencePeerPk(conferenceId, peerId);
    emit core->conferencePeerNameChanged(conferenceId, peerPk, newName);
}

void Core::onConferenceTitle(const Tox_Event_Conference_Title* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto conferenceId = tox_event_conference_title_get_conference_number(event);
    const auto peerId = tox_event_conference_title_get_peer_number(event);
    const auto* const title = tox_event_conference_title_get_title(event);
    const auto length = tox_event_conference_title_get_title_length(event);
    QString author;
    // from tox.h: "If peer_number == UINT32_MAX, then author is unknown (e.g. initial joining the conference)."
    if (peerId != std::numeric_limits<uint32_t>::max()) {
        author = core->getConferencePeerName(conferenceId, peerId);
    }
    emit core->saveRequest();
    emit core->conferenceTitleChanged(conferenceId, author, ToxString(title, length).getQString());
}

void Core::onFriendReadReceipt(const Tox_Event_Friend_Read_Receipt* event, void* vCore)
{
    Core* core = static_cast<Core*>(vCore);
    const auto friendId = tox_event_friend_read_receipt_get_friend_number(event);
    const auto receipt = tox_event_friend_read_receipt_get_message_id(event);
    emit core->receiptReceived(friendId, ReceiptNum{receipt});
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

bool Core::sendMessageWithType(uint32_t friendId, const QString& message, MessageType type,
                               ReceiptNum& receipt)
{
    const int size = message.toUtf8().size();
    const auto maxSize = static_cast<int>(getMaxMessageSize());
    if (size > maxSize) {
        assert(false);
        qCritical() << "Core::sendMessageWithType called with message of size:" << size
                    << "when max is:" << maxSize << ". Ignoring.";
        return false;
    }

    const ToxString cMessage(message);
    Tox_Err_Friend_Send_Message error;
    receipt = ReceiptNum{tox_friend_send_message(tox.get(), friendId, toToxEnum(type),
                                                 cMessage.data(), cMessage.size(), &error)};
    return PARSE_ERR(error);
}

bool Core::sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt)
{
    const QMutexLocker<QRecursiveMutex> ml(&coreLoopLock);
    return sendMessageWithType(friendId, message, MessageType::NORMAL, receipt);
}

bool Core::sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt)
{
    const QMutexLocker<QRecursiveMutex> ml(&coreLoopLock);
    return sendMessageWithType(friendId, action, MessageType::ACTION, receipt);
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

void Core::sendConferenceMessageWithType(int conferenceId, const QString& message, MessageType type)
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
    tox_conference_send_message(tox.get(), conferenceId, toToxEnum(type), cMsg.data(), cMsg.size(),
                                &error);
    if (!PARSE_ERR(error)) {
        emit conferenceSentFailed(conferenceId);
        return;
    }
}

void Core::sendConferenceMessage(int conferenceId, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    sendConferenceMessageWithType(conferenceId, message, MessageType::NORMAL);
}

void Core::sendConferenceAction(int conferenceId, const QString& message)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    sendConferenceMessageWithType(conferenceId, message, MessageType::ACTION);
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
    const ConferenceType confType = inviteInfo.getType();
    const QByteArray invite = inviteInfo.getInvite();
    const auto* const cookie = reinterpret_cast<const uint8_t*>(invite.data());
    const size_t cookieLength = invite.length();
    uint32_t conferenceNum{std::numeric_limits<uint32_t>::max()};
    switch (confType) {
    case ConferenceType::TEXT: {
        qDebug() << "Trying to accept invite for text conference sent by friend" << friendId;
        Tox_Err_Conference_Join error;
        conferenceNum = tox_conference_join(tox.get(), friendId, cookie, cookieLength, &error);
        if (!PARSE_ERR(error)) {
            conferenceNum = std::numeric_limits<uint32_t>::max();
        }
        break;
    }
    case ConferenceType::AV: {
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

int Core::createConference(ConferenceType type)
{
    const QMutexLocker<QRecursiveMutex> ml{&coreLoopLock};

    if (type == ConferenceType::TEXT) {
        Tox_Err_Conference_New error;
        const uint32_t conferenceId = tox_conference_new(tox.get(), &error);
        if (PARSE_ERR(error)) {
            emit saveRequest();
            emit emptyConferenceCreated(conferenceId, getConferencePersistentId(conferenceId));
            return conferenceId;
        }
        return std::numeric_limits<uint32_t>::max();
    }
    if (type == ConferenceType::AV) {
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
