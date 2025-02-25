/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "conferenceid.h"
#include "conferencetype.h"
#include "icoreconferencemessagesender.h"
#include "icoreconferencequery.h"
#include "icorefriendmessagesender.h"
#include "icoreidhandler.h"
#include "receiptnum.h"
#include "toxfile.h"
#include "toxid.h"
#include "toxpk.h"

#include "src/model/status.h"

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>

#include <memory>
#include <tox/tox.h>

class CoreAV;
class CoreFile;
class IAudioControl;
class ICoreSettings;
class ConferenceInvite;
class Profile;
class Core;
class IBootstrapListGenerator;
struct DhtServer;
struct Tox;
struct Tox_Event;
struct Tox_Event_Conference_Invite;
struct Tox_Event_Conference_Message;
struct Tox_Event_Friend_Request;
struct Tox_Event_Friend_Message;
struct Tox_Event_Friend_Name;
struct Tox_Event_Friend_Typing;
struct Tox_Event_Friend_Status_Message;
struct Tox_Event_Friend_Status;
struct Tox_Event_Friend_Connection_Status;
struct Tox_Event_Conference_Peer_List_Changed;
struct Tox_Event_Conference_Peer_Name;
struct Tox_Event_Conference_Title;
struct Tox_Event_Friend_Read_Receipt;

using ToxCorePtr = std::unique_ptr<Core>;

class Core : public QObject,
             public ICoreFriendMessageSender,
             public ICoreIdHandler,
             public ICoreConferenceMessageSender,
             public ICoreConferenceQuery
{
    Q_OBJECT

private:
    struct Tox_Deleter
    {
        void operator()(Tox* tox) const;
    };
    using ToxPtr = std::unique_ptr<Tox, Tox_Deleter>;

public:
    enum class MessageType
    {
        NORMAL,
        ACTION,
    };

    enum class ToxCoreErrors
    {
        OK,
        BAD_PROXY,
        INVALID_SAVE,
        FAILED_TO_START,
        ERROR_ALLOC,
    };

    static std::pair<ToxCorePtr, Core::ToxCoreErrors>
    makeToxCore(const QByteArray& savedata, const ICoreSettings& settings,
                IBootstrapListGenerator& bootstrapNodes);
    const CoreAV* getAv() const;
    CoreAV* getAv();
    void setAv(CoreAV* coreAv);

    CoreFile* getCoreFile() const;
    Tox* getTox() const;
    QRecursiveMutex& getCoreLoopLock() const;

    ~Core() override;

    static const QString TOX_EXT;
    static uint64_t getMaxMessageSize();
    QString getPeerName(const ToxPk& id) const;
    QVector<uint32_t> getFriendList() const;
    ConferenceId getConferencePersistentId(uint32_t conferenceNumber) const override;
    uint32_t getConferenceNumberPeers(int conferenceId) const override;
    QString getConferencePeerName(int conferenceId, int peerId) const override;
    ToxPk getConferencePeerPk(int conferenceId, int peerId) const override;
    QStringList getConferencePeerNames(int conferenceId) const override;
    bool getConferenceAvEnabled(int conferenceId) const override;
    ToxPk getFriendPublicKey(uint32_t friendNumber) const;
    QString getFriendUsername(uint32_t friendNumber) const;

    bool isFriendOnline(uint32_t friendId) const;
    bool hasFriendWithPublicKey(const ToxPk& publicKey) const;
    uint32_t joinConference(const ConferenceInvite& inviteInfo);
    void quitConference(int conferenceId) const;

    QString getUsername() const override;
    Status::Status getStatus() const;
    QString getStatusMessage() const;
    ToxId getSelfId() const override;
    ToxPk getSelfPublicKey() const override;

    QByteArray getSelfDhtId() const;
    int getSelfUdpPort() const;

public slots:
    void start();

    QByteArray getToxSaveData();

    void acceptFriendRequest(const ToxPk& friendPk);
    void requestFriendship(const ToxId& friendId, const QString& message);
    void conferenceInviteFriend(uint32_t friendId, int conferenceId);
    int createConference(ConferenceType type = ConferenceType::AV);

    void removeFriend(uint32_t friendId);
    void removeConference(int conferenceId);

    void setStatus(Status::Status status);
    void setUsername(const QString& username);
    void setStatusMessage(const QString& message);

    bool sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt) override;
    void sendConferenceMessage(int conferenceId, const QString& message) override;
    void sendConferenceAction(int conferenceId, const QString& message) override;
    void changeConferenceTitle(uint32_t conferenceId, const QString& title);
    bool sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt) override;
    void sendTyping(uint32_t friendId, bool typing);

    void setNospam(uint32_t nospam);

signals:
    void connected();
    void disconnected();

    void friendRequestReceived(const ToxPk& friendPk, const QString& message);
    void friendAvatarChanged(const ToxPk& friendPk, const QByteArray& pic);
    void friendAvatarRemoved(const ToxPk& friendPk);

    void requestSent(const ToxPk& friendPk, const QString& message);
    void failedToAddFriend(const ToxPk& friendPk, const QString& errorInfo = QString());

    void usernameSet(const QString& username);
    void statusMessageSet(const QString& message);
    void statusSet(Status::Status status);
    void idSet(const ToxId& id);

    void failedToSetUsername(const QString& username);
    void failedToSetStatusMessage(const QString& message);
    void failedToSetStatus(Status::Status status);
    void failedToSetTyping(bool typing);

    void saveRequest();

    /**
     * @deprecated prefer signals using ToxPk
     */

    void fileAvatarOfferReceived(uint32_t friendId, uint32_t fileId, const QByteArray& avatarHash,
                                 uint64_t filesize);

    void friendMessageReceived(uint32_t friendId, const QString& message, bool isAction);
    void friendAdded(uint32_t friendId, const ToxPk& friendPk);

    void friendStatusChanged(uint32_t friendId, Status::Status status);
    void friendStatusMessageChanged(uint32_t friendId, const QString& message);
    void friendUsernameChanged(uint32_t friendId, const QString& username);
    void friendTypingChanged(uint32_t friendId, bool isTyping);

    void friendRemoved(uint32_t friendId);
    void friendLastSeenChanged(uint32_t friendId, const QDateTime& dateTime);

    void emptyConferenceCreated(uint32_t conferencenumber, ConferenceId conferenceId,
                                const QString& title = QString());
    void conferenceInviteReceived(const ConferenceInvite& inviteInfo);
    void conferenceMessageReceived(uint32_t conferencenumber, uint32_t peernumber,
                                   const QString& message, bool isAction);
    void conferenceNamelistChanged(uint32_t conferencenumber, uint32_t peernumber, uint8_t change);
    void conferencePeerlistChanged(uint32_t conferencenumber);
    void conferencePeerNameChanged(uint32_t conferencenumber, const ToxPk& peerPk,
                                   const QString& newName);
    void conferenceTitleChanged(uint32_t conferencenumber, const QString& author, const QString& title);
    void conferencePeerAudioPlaying(uint32_t conferencenumber, ToxPk peerPk);
    void conferenceSentFailed(uint32_t conferenceId);
    void conferenceJoined(uint32_t conferencenumber, ConferenceId conferenceId);
    void actionSentResult(uint32_t friendId, const QString& action, int success);

    void receiptReceived(uint32_t friendId, ReceiptNum receipt);

    void failedToRemoveFriend(uint32_t friendId);

private:
    Core(QThread* coreThread_, IBootstrapListGenerator& bootstrapListGenerator_,
         const ICoreSettings& settings_);

    static void onFriendRequest(const Tox_Event_Friend_Request* event, void* vCore);
    static void onFriendMessage(const Tox_Event_Friend_Message* event, void* vCore);
    static void onFriendName(const Tox_Event_Friend_Name* event, void* vCore);
    static void onFriendTyping(const Tox_Event_Friend_Typing* event, void* vCore);
    static void onFriendStatusMessage(const Tox_Event_Friend_Status_Message* event, void* vCore);
    static void onFriendStatus(const Tox_Event_Friend_Status* event, void* vCore);
    static void onFriendConnectionStatus(const Tox_Event_Friend_Connection_Status* event, void* vCore);
    static void onConferenceInvite(const Tox_Event_Conference_Invite* event, void* vCore);
    static void onConferenceMessage(const Tox_Event_Conference_Message* event, void* vCore);
    static void onConferencePeerListChanged(const Tox_Event_Conference_Peer_List_Changed* event,
                                            void* vCore);
    static void onConferencePeerName(const Tox_Event_Conference_Peer_Name* event, void* vCore);
    static void onConferenceTitle(const Tox_Event_Conference_Title* event, void* vCore);
    static void onFriendReadReceipt(const Tox_Event_Friend_Read_Receipt* event, void* vCore);

    void sendConferenceMessageWithType(int conferenceId, const QString& message, MessageType type);
    bool sendMessageWithType(uint32_t friendId, const QString& message, MessageType type,
                             ReceiptNum& receipt);
    bool checkConnection();

    void loadFriends();
    void loadConferences();
    void bootstrapDht();

    void checkLastOnline(uint32_t friendId);

    QString getFriendRequestErrorMessage(const ToxId& friendId, const QString& message) const;

    void dispatchEvent(const Tox_Event* event);

private slots:
    void process();
    void onStarted();

private:
/* Using the now commented out statements in checkConnection(), I watched how
 * many ticks disconnects-after-initial-connect lasted. Out of roughly 15 trials,
 * 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was ~50 ticks.
 * So I set the tolerance here at 25, and initial DCs should be very rare now.
 * This should be able to go to 50 or 100 without affecting legitimate disconnects'
 * downtime, but lets be conservative for now. Edit: now ~~40~~ 30.
 */
#define CORE_DISCONNECT_TOLERANCE 30

    ToxPtr tox;

    std::unique_ptr<CoreFile> file;
    CoreAV* av = nullptr;
    QTimer* toxTimer = nullptr;
    // recursive, since we might call our own functions
    mutable QRecursiveMutex coreLoopLock;

    std::unique_ptr<QThread> coreThread;
    const IBootstrapListGenerator& bootstrapListGenerator;
    const ICoreSettings& settings;
    bool isConnected = false;
    int tolerance = CORE_DISCONNECT_TOLERANCE;
};
