/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/icoregroupquery.h"

/**
 * Mock 1 peer at group number 0
 */
class MockGroupQuery : public ICoreGroupQuery
{
public:
    MockGroupQuery() = default;
    virtual ~MockGroupQuery();
    MockGroupQuery(const MockGroupQuery&) = default;
    MockGroupQuery& operator=(const MockGroupQuery&) = default;
    MockGroupQuery(MockGroupQuery&&) = default;
    MockGroupQuery& operator=(MockGroupQuery&&) = default;

    GroupId getGroupPersistentId(uint32_t groupNumber) const override
    {
        std::ignore = groupNumber;
        return GroupId(0);
    }

    uint32_t getGroupNumberPeers(int groupId) const override
    {
        std::ignore = groupId;
        if (emptyGroup) {
            return 1;
        }

        return 2;
    }

    QString getGroupPeerName(int groupId, int peerId) const override
    {
        std::ignore = groupId;
        return QString("peer").append(QString::number(peerId));
    }

    ToxPk getGroupPeerPk(int groupId, int peerId) const override
    {
        std::ignore = groupId;
        uint8_t id[ToxPk::size] = {static_cast<uint8_t>(peerId)};
        return ToxPk(id);
    }

    QStringList getGroupPeerNames(int groupId) const override
    {
        std::ignore = groupId;
        if (emptyGroup) {
            return QStringList({QString("me")});
        }
        return QStringList({QString("me"), QString("other")});
    }

    bool getGroupAvEnabled(int groupId) const override
    {
        std::ignore = groupId;
        return false;
    }

    void setAsEmptyGroup()
    {
        emptyGroup = true;
    }

    void setAsFunctionalGroup()
    {
        emptyGroup = false;
    }

private:
    bool emptyGroup = false;
};
