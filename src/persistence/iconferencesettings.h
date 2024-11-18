/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "util/interface.h"

#include <QStringList>

class IConferenceSettings
{
public:
    IConferenceSettings() = default;
    virtual ~IConferenceSettings();
    IConferenceSettings(const IConferenceSettings&) = default;
    IConferenceSettings& operator=(const IConferenceSettings&) = default;
    IConferenceSettings(IConferenceSettings&&) = default;
    IConferenceSettings& operator=(IConferenceSettings&&) = default;

    virtual QStringList getBlackList() const = 0;
    virtual void setBlackList(const QStringList& blist) = 0;

    virtual bool getShowConferenceJoinLeaveMessages() const = 0;
    virtual void setShowConferenceJoinLeaveMessages(bool newValue) = 0;

    DECLARE_SIGNAL(blackListChanged, QStringList const& blist);
    DECLARE_SIGNAL(showConferenceJoinLeaveMessagesChanged, bool show);
};
