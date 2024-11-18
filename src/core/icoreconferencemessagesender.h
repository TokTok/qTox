/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QString>

class ICoreConferenceMessageSender
{
public:
    ICoreConferenceMessageSender() = default;
    virtual ~ICoreConferenceMessageSender();
    ICoreConferenceMessageSender(const ICoreConferenceMessageSender&) = default;
    ICoreConferenceMessageSender& operator=(const ICoreConferenceMessageSender&) = default;
    ICoreConferenceMessageSender(ICoreConferenceMessageSender&&) = default;
    ICoreConferenceMessageSender& operator=(ICoreConferenceMessageSender&&) = default;

    virtual void sendConferenceAction(int conferenceId, const QString& message) = 0;
    virtual void sendConferenceMessage(int conferenceId, const QString& message) = 0;
};
