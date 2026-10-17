/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/model/status.h"
#include "toxpk.h"

#include <QByteArray>
#include <QString>

#include <cstdint>

enum class GroupRole
{
    Founder = 0,
    Moderator = 1,
    User = 2,
    Observer = 3,
    Unknown = -1,
};

enum class GroupTopicLock
{
    Enabled = 0,
    Disabled = 1,
    Unknown = -1,
};

enum class GroupVoiceState
{
    All = 0,
    Moderator = 1,
    Founder = 2,
    Unknown = -1,
};

enum class GroupPrivacyState
{
    Public = 0,
    Private = 1,
    Unknown = -1,
};

class ICoreGroupQuery
{
public:
    ICoreGroupQuery() = default;
    virtual ~ICoreGroupQuery();
    ICoreGroupQuery(const ICoreGroupQuery&) = default;
    ICoreGroupQuery& operator=(const ICoreGroupQuery&) = default;
    ICoreGroupQuery(ICoreGroupQuery&&) = default;
    ICoreGroupQuery& operator=(ICoreGroupQuery&&) = default;

    virtual QString getGroupPeerName(int groupNumber, int peerId) const = 0;
    virtual ToxPk getGroupPeerPk(int groupNumber, int peerId) const = 0;
    virtual ToxPk getGroupSelfPk(int groupNumber) const = 0;
    virtual QString getGroupTitle(int groupNumber) const = 0;
    virtual QString getGroupTopic(int groupNumber) const = 0;
    virtual QString getGroupSelfName(int groupNumber) const = 0;
    virtual bool setGroupSelfName(int groupNumber, const QString& name) = 0;
    virtual uint32_t getGroupSelfPeerId(int groupNumber) const = 0;
    virtual Status::Status getGroupSelfStatus(int groupNumber) const = 0;
    virtual bool setGroupSelfStatus(int groupNumber, Status::Status status) = 0;
    virtual Status::Status getGroupPeerStatus(int groupNumber, int peerId) const = 0;
    virtual GroupRole getGroupPeerRole(int groupNumber, int peerId) const = 0;
    virtual bool setGroupPeerRole(int groupNumber, int peerId, GroupRole role) = 0;
    virtual bool kickGroupPeer(int groupNumber, int peerId) = 0;
    virtual bool setGroupPassword(int groupNumber, const QByteArray& password) = 0;
    virtual bool setGroupPeerLimit(int groupNumber, uint16_t peerLimit) = 0;
    virtual bool setGroupTopicLock(int groupNumber, GroupTopicLock topicLock) = 0;
    virtual bool setGroupVoiceState(int groupNumber, GroupVoiceState voiceState) = 0;
    virtual bool setGroupPrivacyState(int groupNumber, GroupPrivacyState privacyState) = 0;
    virtual bool getGroupHasPassword(int groupNumber) const = 0;
    virtual uint16_t getGroupPeerLimit(int groupNumber) const = 0;
    virtual GroupTopicLock getGroupTopicLock(int groupNumber) const = 0;
    virtual GroupVoiceState getGroupVoiceState(int groupNumber) const = 0;
    virtual GroupPrivacyState getGroupPrivacyState(int groupNumber) const = 0;
};
