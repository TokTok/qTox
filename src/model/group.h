/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "chat.h"

#include "src/core/chatid.h"
#include "src/core/groupid.h"
#include "src/core/icoregroupquery.h"
#include "src/core/icoreidhandler.h"
#include "src/core/toxpk.h"

#include <QByteArray>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>

class FriendList;

class Group : public Chat
{
    Q_OBJECT
public:
    Group(int groupId_, GroupId persistentGroupId, QString groupName, QString selfName_,
          ICoreGroupQuery& groupQuery_, ICoreIdHandler& idHandler_, FriendList& friendList);
    uint32_t getId() const override;
    const GroupId& getPersistentId() const override;
    void setToxGroupNumber(uint32_t groupNumber);
    int getPeersCount() const;
    const QMap<ToxPk, QString>& getPeerList() const;
    bool peerHasNickname(ToxPk pk);

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void setName(const QString& newTitle) override;
    void updateName(const QString& newTitle);
    QString getName() const;
    QString getDisplayedName() const override;
    QString getDisplayedName(const ToxPk& contact) const override;
    QString resolveToxPk(const ToxPk& id) const;
    ToxPk getSelfPeerPk() const;
    void setSelfName(const QString& name);
    QString getSelfName() const;

    void setTopic(const QString& author, const QString& newTopic);
    QString getTopic() const;

    void setPasswordSet(bool hasPassword);
    bool isPasswordSet() const;
    void setPeerLimit(uint16_t peerLimit);
    uint16_t getPeerLimit() const;
    void setTopicLock(GroupTopicLock topicLock);
    GroupTopicLock getTopicLock() const;
    void setVoiceState(GroupVoiceState voiceState);
    GroupVoiceState getVoiceState() const;
    void setPrivacyState(GroupPrivacyState privacyState);
    GroupPrivacyState getPrivacyState() const;

    bool setGroupPassword(const QByteArray& password);
    bool setGroupPeerLimit(uint16_t peerLimit);
    bool setGroupTopicLock(GroupTopicLock topicLock);
    bool setGroupVoiceState(GroupVoiceState voiceState);
    bool setGroupPrivacyState(GroupPrivacyState privacyState);
    bool setGroupNickname(const QString& nickname);
    QString getGroupNickname() const;
    bool setGroupStatus(Status::Status status);
    Status::Status getGroupStatus() const;

    void onPeerJoin(uint32_t peerId);
    void onPeerExit(uint32_t peerId);
    void clearPeers();
    void onPeerNameChanged(uint32_t peerId, const QString& newName);
    void onPeerStatusChanged(uint32_t peerId, Status::Status status);
    void updatePeerRoles();
    GroupRole getPeerRole(const ToxPk& pk) const;
    Status::Status getPeerStatus(const ToxPk& pk) const;
    bool setPeerRole(const ToxPk& pk, GroupRole role);
    bool kickPeer(const ToxPk& pk);
    ToxPk resolvePeerPk(uint32_t peerId) const;
    uint32_t getPeerId(const ToxPk& pk) const;

signals:
    void titleChanged(const QString& author, const QString& title);
    void titleChangedByUser(const QString& title);
    void topicChanged(const QString& author, const QString& topic);
    void userJoined(const ToxPk& user, const QString& name);
    void userLeft(const ToxPk& user, const QString& name);
    void numPeersChanged(int numPeers);
    void peerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName);
    void peerStatusChanged(const ToxPk& peer, Status::Status status);
    void peerRolesChanged();
    void passwordSetChanged(bool hasPassword);
    void peerLimitChanged(uint16_t peerLimit);
    void topicLockChanged(GroupTopicLock topicLock);
    void voiceStateChanged(GroupVoiceState voiceState);
    void privacyStateChanged(GroupPrivacyState privacyState);
    void nicknameChanged(const QString& nickname);

private:
    QString resolvePeerName(uint32_t peerId) const;

private:
    ICoreGroupQuery& groupQuery;
    ICoreIdHandler& idHandler;
    QString selfName;
    QString groupName;
    QString toxcoreName;
    QString topic;
    QString nickname;
    Status::Status selfStatus = Status::Status::Online;
    bool hasPassword = false;
    uint16_t peerLimit = 0;
    GroupTopicLock topicLock = GroupTopicLock::Unknown;
    GroupVoiceState voiceState = GroupVoiceState::Unknown;
    GroupPrivacyState privacyState = GroupPrivacyState::Unknown;
    QMap<ToxPk, QString> peerDisplayNames;
    QMap<ToxPk, Status::Status> peerStatuses;
    QMap<uint32_t, ToxPk> peerIdToPk;
    QMap<ToxPk, GroupRole> peerRoles;
    bool hasNewMessages;
    bool userWasMentioned;
    int toxGroupNum;
    const GroupId groupId;
    FriendList& friendList;
};
