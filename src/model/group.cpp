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

namespace {
const int MAX_GROUP_NAME_LENGTH = 48;
const int MAX_GROUP_TOPIC_LENGTH = 512;
} // namespace

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
    const QString shortTitle = newTitle.left(MAX_GROUP_NAME_LENGTH);
    if (groupName != shortTitle) {
        groupName = shortTitle;
        emit displayedNameChanged(getDisplayedName());
        emit titleChangedByUser(groupName);
        emit titleChanged(selfName, getDisplayedName());
    }
}

void Group::updateName(const QString& newTitle)
{
    const QString shortTitle = newTitle.left(MAX_GROUP_NAME_LENGTH);
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
    const QString shortTopic = newTopic.left(MAX_GROUP_TOPIC_LENGTH);
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
        const ToxPk self = getSelfPeerPk();
        if (!self.isEmpty()) {
            onPeerNameChanged(self, nameToSet);
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
        const ToxPk self = getSelfPeerPk();
        if (!self.isEmpty()) {
            onPeerStatusChanged(self, status);
        }
        return true;
    }
    return false;
}

Status::Status Group::getGroupStatus() const
{
    return selfStatus;
}

QString Group::displayName(const ToxPk& pk, const QString& peerName) const
{
    if (pk == getSelfPeerPk()) {
        return idHandler.getUsername();
    }
    return friendList.decideNickname(pk, peerName);
}

void Group::onPeerJoin(const ToxPk& pk, const QString& peerName, GroupRole role, Status::Status status)
{
    peerRoles[pk] = role;
    peerStatuses[pk] = status;
    const QString name = displayName(pk, peerName);
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

void Group::onPeerExit(const ToxPk& pk)
{
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
    peerStatuses.clear();
    peerRoles.clear();
    peerDisplayNames.clear();
    emit numPeersChanged(0);
}

void Group::onPeerNameChanged(const ToxPk& pk, const QString& newName)
{
    const QString shownName = friendList.decideNickname(pk, newName);
    if (!peerDisplayNames.contains(pk)) {
        peerDisplayNames[pk] = shownName;
        if (pk == getSelfPeerPk()) {
            selfName = shownName;
        }
        emit userJoined(pk, shownName);
        emit numPeersChanged(peerDisplayNames.size());
        return;
    }

    if (peerDisplayNames[pk] != shownName) {
        const auto oldName = peerDisplayNames[pk];
        peerDisplayNames[pk] = shownName;
        if (pk == getSelfPeerPk()) {
            selfName = shownName;
        }
        emit peerNameChanged(pk, oldName, shownName);
    }
}

void Group::onPeerStatusChanged(const ToxPk& pk, Status::Status status)
{
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
    for (const ToxPk& pk : peerDisplayNames.keys()) {
        const GroupRole role = groupQuery.getGroupPeerRole(toxGroupNum, pk);
        if (peerRoles.value(pk, GroupRole::Unknown) != role) {
            peerRoles[pk] = role;
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
    return groupQuery.setGroupPeerRole(toxGroupNum, pk, role);
}

bool Group::kickPeer(const ToxPk& pk)
{
    if (!groupQuery.kickGroupPeer(toxGroupNum, pk)) {
        return false;
    }
    onPeerExit(pk);
    return true;
}
