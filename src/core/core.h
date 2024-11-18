/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "conferenceid.h"
#include "icorefriendmessagesender.h"
#include "icoreconferencemessagesender.h"
#include "icoreconferencequery.h"
#include "icoreidhandler.h"
#include "receiptnum.h"
#include "toxfile.h"
#include "toxid.h"
#include "toxpk.h"

#include "src/model/status.h"
#include "util/strongtype.h"
#include <tox/tox.h>

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QTimer>

#include <functional>
#include <memory>

class CoreAV;
class CoreFile;
class CoreExt;
class IAudioControl;
class ICoreSettings;
class ConferenceInvite;
class Profile;
class Core;
class IBootstrapListGenerator;
struct DhtServer;

using ToxCorePtr = std::unique_ptr<Core>;

class Core : public QObject,
             public ICoreFriendMessageSender,
             public ICoreIdHandler,
             public ICoreConferenceMessageSender,
             public ICoreConferenceQuery
{
    Q_OBJECT
public:
    enum class ToxCoreErrors
    {
        BAD_PROXY,
        INVALID_SAVE,
        FAILED_TO_START,
        ERROR_ALLOC
    };

    static ToxCorePtr makeToxCore(const QByteArray& savedata, const ICoreSettings& settings,
                                  IBootstrapListGenerator& bootstrapNodes,
                                  ToxCoreErrors* err = nullptr);
    const CoreAV* getAv() const;
    CoreAV* getAv();
    void setAv(CoreAV* coreAv);

    CoreFile* getCoreFile() const;
    Tox* getTox() const;
    QRecursiveMutex& getCoreLoopLock() const;

    const CoreExt* getExt() const;
    CoreExt* getExt();
    ~Core();

    static const QString TOX_EXT;
    uint64_t getMaxMessageSize() const;
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
    int createConference(uint8_t type = TOX_CONFERENCE_TYPE_AV);

    void removeFriend(uint32_t friendId);
    void removeConference(int conferenceId);

    void setStatus(Status::Status status);
    void setUsername(const QString& username);
    void setStatusMessage(const QString& message);

    bool sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt) override;
    void sendConferenceMessage(int conferenceId, const QString& message) override;
    void sendConferenceAction(int conferenceId, const QString& message) override;
    void changeConferenceTitle(int conferenceId, const QString& title);
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

    void emptyConferenceCreated(int conferencenumber, const ConferenceId conferenceId, const QString& title = QString());
    void conferenceInviteReceived(const ConferenceInvite& inviteInfo);
    void conferenceMessageReceived(int conferencenumber, int peernumber, const QString& message, bool isAction);
    void conferenceNamelistChanged(int conferencenumber, int peernumber, uint8_t change);
    void conferencePeerlistChanged(int conferencenumber);
    void conferencePeerNameChanged(int conferencenumber, const ToxPk& peerPk, const QString& newName);
    void conferenceTitleChanged(int conferencenumber, const QString& author, const QString& title);
    void conferencePeerAudioPlaying(int conferencenumber, ToxPk peerPk);
    void conferenceSentFailed(int conferenceId);
    void conferenceJoined(int conferencenumber, ConferenceId conferenceId);
    void actionSentResult(uint32_t friendId, const QString& action, int success);

    void receiptRecieved(int friedId, ReceiptNum receipt);

    void failedToRemoveFriend(uint32_t friendId);

private:
    Core(QThread* coreThread_, IBootstrapListGenerator& bootstrapListGenerator_,
         const ICoreSettings& settings_);

    static void onFriendRequest(Tox* tox, const uint8_t* cFriendPk, const uint8_t* cMessage,
                                size_t cMessageSize, void* core);
    static void onFriendMessage(Tox* tox, uint32_t friendId, Tox_Message_Type type,
                                const uint8_t* cMessage, size_t cMessageSize, void* core);
    static void onFriendNameChange(Tox* tox, uint32_t friendId, const uint8_t* cName,
                                   size_t cNameSize, void* core);
    static void onFriendTypingChange(Tox* tox, uint32_t friendId, bool isTyping, void* core);
    static void onStatusMessageChanged(Tox* tox, uint32_t friendId, const uint8_t* cMessage,
                                       size_t cMessageSize, void* core);
    static void onUserStatusChanged(Tox* tox, uint32_t friendId, Tox_User_Status userstatus,
                                    void* core);
    static void onConnectionStatusChanged(Tox* tox, uint32_t friendId, Tox_Connection status,
                                          void* vCore);
    static void onConferenceInvite(Tox* tox, uint32_t friendId, Tox_Conference_Type type,
                              const uint8_t* cookie, size_t length, void* vCore);
    static void onConferenceMessage(Tox* tox, uint32_t conferenceId, uint32_t peerId, Tox_Message_Type type,
                               const uint8_t* cMessage, size_t length, void* vCore);
    static void onConferencePeerListChange(Tox* tox, uint32_t conferenceId, void* core);
    static void onConferencePeerNameChange(Tox* tox, uint32_t conferenceId, uint32_t peerId,
                                      const uint8_t* name, size_t length, void* core);
    static void onConferenceTitleChange(Tox* tox, uint32_t conferenceId, uint32_t peerId,
                                   const uint8_t* cTitle, size_t length, void* vCore);

    static void onLosslessPacket(Tox* tox, uint32_t friendId, const uint8_t* data, size_t length,
                                 void* core);
    static void onReadReceiptCallback(Tox* tox, uint32_t friendId, uint32_t receipt, void* core);

    void sendConferenceMessageWithType(int conferenceId, const QString& message, Tox_Message_Type type);
    bool sendMessageWithType(uint32_t friendId, const QString& message, Tox_Message_Type type,
                             ReceiptNum& receipt);
    bool checkConnection();

    void makeTox(QByteArray savedata, ICoreSettings* s);
    void loadFriends();
    void loadConferences();
    void bootstrapDht();

    void checkLastOnline(uint32_t friendId);

    QString getFriendRequestErrorMessage(const ToxId& friendId, const QString& message) const;
    static void registerCallbacks(Tox* tox);

private slots:
    void process();
    void onStarted();

private:
    struct ToxDeleter
    {
        void operator()(Tox* tox_)
        {
            tox_kill(tox_);
        }
    };
/* Using the now commented out statements in checkConnection(), I watched how
 * many ticks disconnects-after-initial-connect lasted. Out of roughly 15 trials,
 * 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was ~50 ticks.
 * So I set the tolerance here at 25, and initial DCs should be very rare now.
 * This should be able to go to 50 or 100 without affecting legitimate disconnects'
 * downtime, but lets be conservative for now. Edit: now ~~40~~ 30.
 */
#define CORE_DISCONNECT_TOLERANCE 30

    using ToxPtr = std::unique_ptr<Tox, ToxDeleter>;
    ToxPtr tox;

    std::unique_ptr<CoreFile> file;
    CoreAV* av = nullptr;
    std::unique_ptr<CoreExt> ext;
    QTimer* toxTimer = nullptr;
    // recursive, since we might call our own functions
    mutable QRecursiveMutex coreLoopLock;

    std::unique_ptr<QThread> coreThread;
    const IBootstrapListGenerator& bootstrapListGenerator;
    const ICoreSettings& settings;
    bool isConnected = false;
    int tolerance = CORE_DISCONNECT_TOLERANCE;
};
