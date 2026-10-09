/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "group.h"

#include "src/core/chatid.h"
#include "src/core/groupid.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"

#include <QDebug>

#include <cassert>
#include <utility>

Group::Group(int groupId_, const GroupId persistentGroupId, QString name, QString selfName_,
             ICoreGroupQuery& groupQuery_, ICoreIdHandler& idHandler_, FriendList& friendList_)
    : groupQuery(groupQuery_)
    , idHandler(idHandler_)
    , selfName{std::move(selfName_)}
    , toxcoreName{std::move(name)}
    , toxGroupNum(groupId_)
    , groupId{persistentGroupId}
    , friendList{friendList_}
{
    hasNewMessages = false;
    userWasMentioned = false;
}

void Group::setName(const QString& newTitle)
{
    const QString shortTitle = newTitle.left(TOX_GROUP_MAX_GROUP_NAME_LENGTH);
    if (groupName != shortTitle) {
        groupName = shortTitle;
        emit displayedNameChanged(getDisplayedName());
        emit titleChangedByUser(groupName);
        emit titleChanged(selfName, getDisplayedName());
    }
}

void Group::updateName(const QString& newTitle)
{
    const QString shortTitle = newTitle.left(TOX_GROUP_MAX_GROUP_NAME_LENGTH);
    if (!shortTitle.isEmpty() && toxcoreName != shortTitle) {
        toxcoreName = shortTitle;
        emit displayedNameChanged(getDisplayedName());
        emit titleChanged(selfName, getDisplayedName());
    }
}

QString Group::getName() const
{
    return groupName;
}

QString Group::getDisplayedName() const
{
    if (!groupName.isEmpty()) {
        return groupName;
    }
    if (!toxcoreName.isEmpty()) {
        return toxcoreName;
    }
    return tr("Group %1").arg(groupId.toString().left(8));
}

QString Group::getDisplayedName(const ToxPk& contact) const
{
    return resolveToxPk(contact);
}

uint32_t Group::getId() const
{
    return toxGroupNum;
}

void Group::setToxGroupNumber(uint32_t groupNumber)
{
    toxGroupNum = groupNumber;
}

const GroupId& Group::getPersistentId() const
{
    return groupId;
}

int Group::getPeersCount() const
{
    return peerDisplayNames.size();
}

/**
 * @brief Gets the PKs and names of all peers
 * @return PKs and names of all peers, including our own PK and name
 */
const QMap<ToxPk, QString>& Group::getPeerList() const
{
    return peerDisplayNames;
}

bool Group::peerHasNickname(ToxPk pk)
{
    return peerDisplayNames.contains(pk);
}

void Group::setEventFlag(bool f)
{
    hasNewMessages = f;
}

bool Group::getEventFlag() const
{
    return hasNewMessages;
}

void Group::setMentionedFlag(bool f)
{
    userWasMentioned = f;
}

bool Group::getMentionedFlag() const
{
    return userWasMentioned;
}

QString Group::resolveToxPk(const ToxPk& id) const
{
    auto it = peerDisplayNames.find(id);

    if (it != peerDisplayNames.end()) {
        return *it;
    }

    return {};
}

ToxPk Group::getSelfPeerPk() const
{
    return groupQuery.getGroupSelfPk(toxGroupNum);
}

void Group::setSelfName(const QString& name)
{
    selfName = name;
}

QString Group::getSelfName() const
{
    return selfName;
}

void Group::setTopic(const QString& author, const QString& newTopic)
{
    const QString shortTopic = newTopic.left(TOX_GROUP_MAX_TOPIC_LENGTH);
    if (topic != shortTopic) {
        topic = shortTopic;
        emit topicChanged(author, topic);
    }
}

QString Group::getTopic() const
{
    return topic;
}

void Group::setPasswordSet(bool hasPassword_)
{
    if (hasPassword != hasPassword_) {
        hasPassword = hasPassword_;
        emit passwordSetChanged(hasPassword);
    }
}

bool Group::isPasswordSet() const
{
    return hasPassword;
}

void Group::setPeerLimit(uint16_t peerLimit_)
{
    if (peerLimit != peerLimit_) {
        peerLimit = peerLimit_;
        emit peerLimitChanged(peerLimit);
    }
}

uint16_t Group::getPeerLimit() const
{
    return peerLimit;
}

void Group::setTopicLock(GroupTopicLock topicLock_)
{
    if (topicLock != topicLock_) {
        topicLock = topicLock_;
        emit topicLockChanged(topicLock);
    }
}

GroupTopicLock Group::getTopicLock() const
{
    return topicLock;
}

void Group::setVoiceState(GroupVoiceState voiceState_)
{
    if (voiceState != voiceState_) {
        voiceState = voiceState_;
        emit voiceStateChanged(voiceState);
    }
}

GroupVoiceState Group::getVoiceState() const
{
    return voiceState;
}

void Group::setPrivacyState(GroupPrivacyState privacyState_)
{
    if (privacyState != privacyState_) {
        privacyState = privacyState_;
        emit privacyStateChanged(privacyState);
    }
}

GroupPrivacyState Group::getPrivacyState() const
{
    return privacyState;
}

bool Group::setGroupPassword(const QByteArray& password)
{
    return groupQuery.setGroupPassword(toxGroupNum, password);
}

bool Group::setGroupPeerLimit(uint16_t peerLimit_)
{
    return groupQuery.setGroupPeerLimit(toxGroupNum, peerLimit_);
}

bool Group::setGroupTopicLock(GroupTopicLock topicLock_)
{
    return groupQuery.setGroupTopicLock(toxGroupNum, topicLock_);
}

bool Group::setGroupVoiceState(GroupVoiceState voiceState_)
{
    return groupQuery.setGroupVoiceState(toxGroupNum, voiceState_);
}

bool Group::setGroupPrivacyState(GroupPrivacyState privacyState_)
{
    return groupQuery.setGroupPrivacyState(toxGroupNum, privacyState_);
}

bool Group::setGroupNickname(const QString& nickname_)
{
    const QString nameToSet = nickname_.isEmpty() ? idHandler.getUsername() : nickname_;
    if (groupQuery.setGroupSelfName(toxGroupNum, nameToSet)) {
        nickname = nickname_;
        emit nicknameChanged(nickname);
        const uint32_t selfPeerId = groupQuery.getGroupSelfPeerId(toxGroupNum);
        if (selfPeerId != std::numeric_limits<uint32_t>::max()) {
            onPeerNameChanged(selfPeerId, nameToSet);
        }
        return true;
    }
    return false;
}

QString Group::getGroupNickname() const
{
    return nickname;
}

bool Group::setGroupStatus(Status::Status status)
{
    if (groupQuery.setGroupSelfStatus(toxGroupNum, status)) {
        selfStatus = status;
        const uint32_t selfPeerId = groupQuery.getGroupSelfPeerId(toxGroupNum);
        if (selfPeerId != std::numeric_limits<uint32_t>::max()) {
            onPeerStatusChanged(selfPeerId, status);
        }
        return true;
    }
    return false;
}

Status::Status Group::getGroupStatus() const
{
    return selfStatus;
}

QString Group::resolvePeerName(uint32_t peerId) const
{
    const ToxPk pk = groupQuery.getGroupPeerPk(toxGroupNum, peerId);
    if (pk == getSelfPeerPk()) {
        return idHandler.getUsername();
    }

    const QString peerName = groupQuery.getGroupPeerName(toxGroupNum, peerId);
    return friendList.decideNickname(pk, peerName);
}

void Group::onPeerJoin(uint32_t peerId)
{
    const ToxPk pk = groupQuery.getGroupPeerPk(toxGroupNum, peerId);
    peerIdToPk[peerId] = pk;
    peerRoles[pk] = groupQuery.getGroupPeerRole(toxGroupNum, peerId);
    peerStatuses[pk] = groupQuery.getGroupPeerStatus(toxGroupNum, peerId);
    const QString name = resolvePeerName(peerId);
    if (peerDisplayNames.contains(pk)) {
        if (peerDisplayNames[pk] != name) {
            const QString oldName = peerDisplayNames[pk];
            peerDisplayNames[pk] = name;
            emit peerNameChanged(pk, oldName, name);
        }
        return;
    }

    peerDisplayNames[pk] = name;
    emit userJoined(pk, name);
    emit numPeersChanged(peerDisplayNames.size());
}

void Group::onPeerExit(uint32_t peerId)
{
    const ToxPk pk = resolvePeerPk(peerId);
    peerIdToPk.remove(peerId);
    peerStatuses.remove(pk);
    peerRoles.remove(pk);
    auto it = peerDisplayNames.find(pk);
    if (it == peerDisplayNames.end()) {
        return;
    }

    const QString name = it.value();
    peerDisplayNames.erase(it);
    emit userLeft(pk, name);
    emit numPeersChanged(peerDisplayNames.size());
}

void Group::clearPeers()
{
    peerIdToPk.clear();
    peerStatuses.clear();
    peerRoles.clear();
    peerDisplayNames.clear();
    emit numPeersChanged(0);
}

void Group::onPeerNameChanged(uint32_t peerId, const QString& newName)
{
    const ToxPk pk = groupQuery.getGroupPeerPk(toxGroupNum, peerId);
    peerIdToPk[peerId] = pk;

    const QString displayName = friendList.decideNickname(pk, newName);
    if (!peerDisplayNames.contains(pk)) {
        peerDisplayNames[pk] = displayName;
        if (pk == getSelfPeerPk()) {
            selfName = displayName;
        }
        emit userJoined(pk, displayName);
        emit numPeersChanged(peerDisplayNames.size());
        return;
    }

    if (peerDisplayNames[pk] != displayName) {
        const auto oldName = peerDisplayNames[pk];
        peerDisplayNames[pk] = displayName;
        if (pk == getSelfPeerPk()) {
            selfName = displayName;
        }
        emit peerNameChanged(pk, oldName, displayName);
    }
}

void Group::onPeerStatusChanged(uint32_t peerId, Status::Status status)
{
    const ToxPk pk = groupQuery.getGroupPeerPk(toxGroupNum, peerId);
    peerIdToPk[peerId] = pk;

    if (pk == getSelfPeerPk()) {
        selfStatus = status;
    }

    if (peerStatuses.value(pk, Status::Status::Online) != status) {
        peerStatuses[pk] = status;
        emit peerStatusChanged(pk, status);
    }
}

void Group::updatePeerRoles()
{
    bool changed = false;
    for (auto it = peerIdToPk.cbegin(); it != peerIdToPk.cend(); ++it) {
        const GroupRole role = groupQuery.getGroupPeerRole(toxGroupNum, it.key());
        if (peerRoles.value(it.value(), GroupRole::Unknown) != role) {
            peerRoles[it.value()] = role;
            changed = true;
        }
    }
    if (changed) {
        emit peerRolesChanged();
    }
}

GroupRole Group::getPeerRole(const ToxPk& pk) const
{
    return peerRoles.value(pk, GroupRole::User);
}

Status::Status Group::getPeerStatus(const ToxPk& pk) const
{
    return peerStatuses.value(pk, Status::Status::Online);
}

bool Group::setPeerRole(const ToxPk& pk, GroupRole role)
{
    for (auto it = peerIdToPk.cbegin(); it != peerIdToPk.cend(); ++it) {
        if (it.value() == pk) {
            return groupQuery.setGroupPeerRole(toxGroupNum, it.key(), role);
        }
    }
    qWarning() << "setPeerRole: unknown peer" << pk.toString();
    return false;
}

bool Group::kickPeer(const ToxPk& pk)
{
    for (auto it = peerIdToPk.cbegin(); it != peerIdToPk.cend(); ++it) {
        if (it.value() == pk) {
            if (groupQuery.kickGroupPeer(toxGroupNum, it.key())) {
                onPeerExit(it.key());
                return true;
            }
            return false;
        }
    }
    qWarning() << "kickPeer: unknown peer" << pk.toString();
    return false;
}

ToxPk Group::resolvePeerPk(uint32_t peerId) const
{
    return peerIdToPk.value(peerId, ToxPk{});
}

uint32_t Group::getPeerId(const ToxPk& pk) const
{
    for (auto it = peerIdToPk.cbegin(); it != peerIdToPk.cend(); ++it) {
        if (it.value() == pk) {
            return it.key();
        }
    }
    return std::numeric_limits<uint32_t>::max();
}
