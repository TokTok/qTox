/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "conferenceid.h"
#include "groupid.h"
#include "icoreconferencemessagesender.h"
#include "icoreconferencequery.h"
#include "icorefriendmessagesender.h"
#include "icoregroupmessagesender.h"
#include "icoregroupquery.h"
#include "icoreidhandler.h"
#include "receiptnum.h"
#include "toxfile.h"
#include "toxid.h"
#include "toxpk.h"

#include "src/model/conferenceinvite.h"
#include "src/model/status.h"

#include <QHash>
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
class GroupInvite;
class Profile;
class Core;
class IBootstrapListGenerator;
struct DhtServer;

using ToxCorePtr = std::unique_ptr<Core>;

class Core : public QObject,
             public ICoreFriendMessageSender,
             public ICoreIdHandler,
             public ICoreConferenceMessageSender,
             public ICoreConferenceQuery,
             public ICoreGroupMessageSender,
             public ICoreGroupQuery
{
    Q_OBJECT
public:
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

    uint32_t getGroupNumberPeers(int groupNumber) const;
    uint32_t getGroupSelfPeerId(int groupNumber) const override;
    QString getGroupPeerName(int groupNumber, int peerId) const override;
    ToxPk getGroupPeerPk(int groupNumber, int peerId) const override;
    ToxPk getGroupSelfPk(int groupNumber) const override;
    QString getGroupTitle(int groupNumber) const override;
    QString getGroupTopic(int groupNumber) const override;
    QString getGroupSelfName(int groupNumber) const override;
    bool setGroupSelfName(int groupNumber, const QString& name) override;
    Status::Status getGroupSelfStatus(int groupNumber) const override;
    bool setGroupSelfStatus(int groupNumber, Status::Status status) override;
    Status::Status getGroupPeerStatus(int groupNumber, int peerId) const override;
    GroupRole getGroupPeerRole(int groupNumber, int peerId) const override;
    bool setGroupPeerRole(int groupNumber, int peerId, GroupRole role) override;
    bool kickGroupPeer(int groupNumber, int peerId) override;
    GroupId getGroupPersistentId(uint32_t groupNumber) const;
    bool reconnectGroup(uint32_t groupNumber);

    bool isFriendOnline(uint32_t friendId) const;
    bool hasFriendWithPublicKey(const ToxPk& publicKey) const;
    uint32_t joinConference(const ConferenceInvite& inviteInfo);
    void quitConference(int conferenceId) const;

    uint32_t joinGroup(const GroupInvite& inviteInfo);
    int joinGroup(const GroupId& groupId);

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

    void groupInviteFriend(uint32_t friendId, int groupNumber);
    int createGroup(const QString& groupName);
    void quitGroup(int groupNumber);
    void changeGroupTopic(uint32_t groupNumber, const QString& topic);
    bool setGroupPassword(int groupNumber, const QByteArray& password) override;
    bool setGroupPeerLimit(int groupNumber, uint16_t peerLimit) override;
    bool setGroupTopicLock(int groupNumber, GroupTopicLock topicLock) override;
    bool setGroupVoiceState(int groupNumber, GroupVoiceState voiceState) override;
    bool setGroupPrivacyState(int groupNumber, GroupPrivacyState privacyState) override;
    bool getGroupHasPassword(int groupNumber) const override;
    uint16_t getGroupPeerLimit(int groupNumber) const override;
    GroupTopicLock getGroupTopicLock(int groupNumber) const override;
    GroupVoiceState getGroupVoiceState(int groupNumber) const override;
    GroupPrivacyState getGroupPrivacyState(int groupNumber) const override;

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

    void sendGroupMessage(uint32_t groupNumber, const QString& message) override;
    void sendGroupAction(uint32_t groupNumber, const QString& message) override;
    void sendGroupPrivateMessage(uint32_t groupNumber, uint32_t peerId,
                                 const QString& message, Tox_Message_Type type) override;

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

    void emptyGroupCreated(uint32_t groupNumber, GroupId groupId, const QString& groupName);
    void groupInviteReceived(const GroupInvite& inviteInfo);
    void groupMessageReceived(uint32_t groupNumber, uint32_t peerId, const QString& message,
                              bool isAction);
    void groupPrivateMessageReceived(uint32_t groupNumber, uint32_t peerId, const QString& message,
                                      bool isAction);
    void groupPeerJoined(uint32_t groupNumber, uint32_t peerId);
    void groupPeerExited(uint32_t groupNumber, uint32_t peerId);
    void groupPeerNameChanged(uint32_t groupNumber, uint32_t peerId, const QString& newName);
    void groupPeerStatusChanged(uint32_t groupNumber, uint32_t peerId, Status::Status status);
    void groupTitleChanged(uint32_t groupNumber, const QString& author, const QString& title);
    void groupTopicChanged(uint32_t groupNumber, const QString& topic);
    void groupSentFailed(uint32_t groupNumber);
    void groupJoined(uint32_t groupNumber, GroupId groupId);
    void groupSelfJoined(uint32_t groupNumber);
    void groupSelfDisconnected(uint32_t groupNumber);
    void groupJoinFailed(uint32_t groupNumber, Tox_Group_Join_Fail failType);
    void groupPeerRolesChanged(uint32_t groupNumber);
    void groupPasswordChanged(uint32_t groupNumber, bool hasPassword);
    void groupPeerLimitChanged(uint32_t groupNumber, uint16_t peerLimit);
    void groupTopicLockChanged(uint32_t groupNumber, GroupTopicLock topicLock);
    void groupVoiceStateChanged(uint32_t groupNumber, GroupVoiceState voiceState);
    void groupPrivacyStateChanged(uint32_t groupNumber, GroupPrivacyState privacyState);

    void receiptReceived(uint32_t friendId, ReceiptNum receipt);

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
    static void onConferenceMessage(Tox* tox, uint32_t conferenceId, uint32_t peerId,
                                    Tox_Message_Type type, const uint8_t* cMessage, size_t length,
                                    void* vCore);
    static void onConferencePeerListChange(Tox* tox, uint32_t conferenceId, void* core);
    static void onConferencePeerNameChange(Tox* tox, uint32_t conferenceId, uint32_t peerId,
                                           const uint8_t* name, size_t length, void* core);
    static void onConferenceTitleChange(Tox* tox, uint32_t conferenceId, uint32_t peerId,
                                        const uint8_t* cTitle, size_t length, void* vCore);

    static void onGroupInvite(Tox* tox, uint32_t friendId, const uint8_t* inviteData,
                              size_t length, const uint8_t* groupName, size_t groupNameLength,
                              void* vCore);
    static void onGroupMessage(Tox* tox, uint32_t groupNumber, uint32_t peerId,
                               Tox_Message_Type type, const uint8_t* cMessage, size_t length,
                               Tox_Group_Message_Id messageId, void* vCore);
    static void onGroupPrivateMessage(Tox* tox, uint32_t groupNumber, uint32_t peerId,
                                       Tox_Message_Type type, const uint8_t* cMessage, size_t length,
                                       Tox_Group_Message_Id messageId, void* vCore);
    static void onGroupPeerJoin(Tox* tox, uint32_t groupNumber, uint32_t peerId, void* vCore);
    static void onGroupPeerExit(Tox* tox, uint32_t groupNumber, uint32_t peerId,
                                Tox_Group_Exit_Type exitType, const uint8_t* name, size_t nameLength,
                                const uint8_t* partMessage, size_t partMessageLength, void* vCore);
    static void onGroupPeerNameChange(Tox* tox, uint32_t groupNumber, uint32_t peerId,
                                      const uint8_t* name, size_t length, void* vCore);
    static void onGroupPeerStatusChange(Tox* tox, uint32_t groupNumber, uint32_t peerId,
                                        Tox_User_Status status, void* vCore);
    static void onGroupSelfJoin(Tox* tox, uint32_t groupNumber, void* vCore);
    static void onGroupTopic(Tox* tox, uint32_t groupNumber, uint32_t peerId, const uint8_t* topic,
                             size_t length, void* vCore);
    static void onGroupJoinFail(Tox* tox, uint32_t groupNumber, Tox_Group_Join_Fail failType,
                                void* vCore);
    static void onGroupModeration(Tox* tox, uint32_t groupNumber, uint32_t sourcePeerId,
                                  uint32_t targetPeerId, Tox_Group_Mod_Event modType, void* vCore);
    static void onGroupPassword(Tox* tox, uint32_t groupNumber, const uint8_t* password,
                                size_t length, void* vCore);
    static void onGroupPeerLimit(Tox* tox, uint32_t groupNumber, uint32_t peerLimit, void* vCore);
    static void onGroupTopicLock(Tox* tox, uint32_t groupNumber, Tox_Group_Topic_Lock topicLock,
                                 void* vCore);
    static void onGroupVoiceState(Tox* tox, uint32_t groupNumber, Tox_Group_Voice_State voiceState,
                                  void* vCore);
    static void onGroupPrivacyState(Tox* tox, uint32_t groupNumber,
                                    Tox_Group_Privacy_State privacyState, void* vCore);

    static void onReadReceiptCallback(Tox* tox, uint32_t friendId, uint32_t receipt, void* core);

    void sendConferenceMessageWithType(int conferenceId, const QString& message, Tox_Message_Type type);
    void sendGroupMessageWithType(uint32_t groupNumber, const QString& message, Tox_Message_Type type);
    bool sendMessageWithType(uint32_t friendId, const QString& message, Tox_Message_Type type,
                             ReceiptNum& receipt);
    bool checkConnection();

    void makeTox(QByteArray savedata, ICoreSettings* s);
    void loadFriends();
    void loadConferences();
    void loadGroups();
    void bootstrapDht();

    void checkLastOnline(uint32_t friendId);

    void startGroupReconnectTimer(uint32_t groupNumber);
    void stopGroupReconnectTimer(uint32_t groupNumber);
    void retryGroupReconnect(uint32_t groupNumber);

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
    QTimer* toxTimer = nullptr;
    // recursive, since we might call our own functions
    mutable QRecursiveMutex coreLoopLock;

    std::unique_ptr<QThread> coreThread;
    const IBootstrapListGenerator& bootstrapListGenerator;
    const ICoreSettings& settings;
    bool isConnected = false;
    int tolerance = CORE_DISCONNECT_TOLERANCE;

    QHash<uint32_t, GroupId> numberToGroupId;
    QHash<GroupId, uint32_t> groupIdToNumber;
    QHash<uint32_t, QTimer*> groupReconnectTimers;
    // number of group members other than ourselves
    QHash<uint32_t, uint32_t> groupPeerCounts;
};
