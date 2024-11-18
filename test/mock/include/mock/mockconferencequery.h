/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/icoreconferencequery.h"

/**
 * Mock 1 peer at conference number 0
 */
class MockConferenceQuery : public ICoreConferenceQuery
{
public:
    MockConferenceQuery() = default;
    virtual ~MockConferenceQuery();
    MockConferenceQuery(const MockConferenceQuery&) = default;
    MockConferenceQuery& operator=(const MockConferenceQuery&) = default;
    MockConferenceQuery(MockConferenceQuery&&) = default;
    MockConferenceQuery& operator=(MockConferenceQuery&&) = default;

    ConferenceId getConferencePersistentId(uint32_t conferenceNumber) const override
    {
        std::ignore = conferenceNumber;
        return ConferenceId(0);
    }

    uint32_t getConferenceNumberPeers(int conferenceId) const override
    {
        std::ignore = conferenceId;
        if (emptyConference) {
            return 1;
        }

        return 2;
    }

    QString getConferencePeerName(int conferenceId, int peerId) const override
    {
        std::ignore = conferenceId;
        return QString("peer").append(QString::number(peerId));
    }

    ToxPk getConferencePeerPk(int conferenceId, int peerId) const override
    {
        std::ignore = conferenceId;
        uint8_t id[ToxPk::size] = {static_cast<uint8_t>(peerId)};
        return ToxPk(id);
    }

    QStringList getConferencePeerNames(int conferenceId) const override
    {
        std::ignore = conferenceId;
        if (emptyConference) {
            return QStringList({QString("me")});
        }
        return QStringList({QString("me"), QString("other")});
    }

    bool getConferenceAvEnabled(int conferenceId) const override
    {
        std::ignore = conferenceId;
        return false;
    }

    void setAsEmptyConference()
    {
        emptyConference = true;
    }

    void setAsFunctionalConference()
    {
        emptyConference = false;
    }

private:
    bool emptyConference = false;
};
