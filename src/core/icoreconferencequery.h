/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "conferenceid.h"
#include "toxpk.h"

#include <QString>
#include <QStringList>

#include <cstdint>

class ICoreConferenceQuery
{
public:
    ICoreConferenceQuery() = default;
    virtual ~ICoreConferenceQuery();
    ICoreConferenceQuery(const ICoreConferenceQuery&) = default;
    ICoreConferenceQuery& operator=(const ICoreConferenceQuery&) = default;
    ICoreConferenceQuery(ICoreConferenceQuery&&) = default;
    ICoreConferenceQuery& operator=(ICoreConferenceQuery&&) = default;

    virtual ConferenceId getConferencePersistentId(uint32_t conferenceNumber) const = 0;
    virtual uint32_t getConferenceNumberPeers(int conferenceId) const = 0;
    virtual QString getConferencePeerName(int conferenceId, int peerId) const = 0;
    virtual ToxPk getConferencePeerPk(int conferenceId, int peerId) const = 0;
    virtual QStringList getConferencePeerNames(int conferenceId) const = 0;
    virtual bool getConferenceAvEnabled(int conferenceId) const = 0;
};
