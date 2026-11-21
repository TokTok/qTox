/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/core/icoregroupquery.h"

class MockGroupQuery : public ICoreGroupQuery
{
public:
    MockGroupQuery() = default;
    ~MockGroupQuery() override;
    MockGroupQuery(const MockGroupQuery&) = default;
    MockGroupQuery& operator=(const MockGroupQuery&) = default;
    MockGroupQuery(MockGroupQuery&&) = default;
    MockGroupQuery& operator=(MockGroupQuery&&) = default;

    ToxPk getGroupSelfPk(int groupNumber) const override
    {
        std::ignore = groupNumber;
        uint8_t id[ToxPk::size] = {static_cast<uint8_t>(0)};
        return ToxPk(id);
    }

    QString getGroupTitle(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return {"group"};
    }

    QString getGroupTopic(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return {};
    }

    QString getGroupSelfName(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return {"self"};
    }

    bool setGroupSelfName(int groupNumber, const QString& name) override
    {
        std::ignore = groupNumber;
        std::ignore = name;
        return true;
    }

    Status::Status getGroupSelfStatus(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return Status::Status::Online;
    }

    bool setGroupSelfStatus(int groupNumber, Status::Status status) override
    {
        std::ignore = groupNumber;
        std::ignore = status;
        return true;
    }

    GroupRole getGroupPeerRole(int groupNumber, const ToxPk& peer) const override
    {
        std::ignore = groupNumber;
        std::ignore = peer;
        return GroupRole::User;
    }

    bool setGroupPeerRole(int groupNumber, const ToxPk& peer, GroupRole role) override
    {
        std::ignore = groupNumber;
        std::ignore = peer;
        std::ignore = role;
        return true;
    }

    bool kickGroupPeer(int groupNumber, const ToxPk& peer) override
    {
        std::ignore = groupNumber;
        std::ignore = peer;
        return true;
    }

    bool setGroupPassword(int groupNumber, const QByteArray& password) override
    {
        std::ignore = groupNumber;
        std::ignore = password;
        return true;
    }

    bool setGroupPeerLimit(int groupNumber, uint16_t peerLimit) override
    {
        std::ignore = groupNumber;
        std::ignore = peerLimit;
        return true;
    }

    bool setGroupTopicLock(int groupNumber, GroupTopicLock topicLock) override
    {
        std::ignore = groupNumber;
        std::ignore = topicLock;
        return true;
    }

    bool setGroupVoiceState(int groupNumber, GroupVoiceState voiceState) override
    {
        std::ignore = groupNumber;
        std::ignore = voiceState;
        return true;
    }

    bool setGroupPrivacyState(int groupNumber, GroupPrivacyState privacyState) override
    {
        std::ignore = groupNumber;
        std::ignore = privacyState;
        return true;
    }

    bool getGroupHasPassword(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return false;
    }

    uint16_t getGroupPeerLimit(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return 0;
    }

    GroupTopicLock getGroupTopicLock(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return GroupTopicLock::Unknown;
    }

    GroupVoiceState getGroupVoiceState(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return GroupVoiceState::Unknown;
    }

    GroupPrivacyState getGroupPrivacyState(int groupNumber) const override
    {
        std::ignore = groupNumber;
        return GroupPrivacyState::Unknown;
    }
};
