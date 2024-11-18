/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

class ChatId;
class ConferenceId;
class ToxPk;

class IDialogs
{
public:
    IDialogs() = default;
    virtual ~IDialogs();
    IDialogs(const IDialogs&) = default;
    IDialogs& operator=(const IDialogs&) = default;
    IDialogs(IDialogs&&) = default;
    IDialogs& operator=(IDialogs&&) = default;

    virtual bool hasChat(const ChatId& chatId) const = 0;
    virtual bool isChatActive(const ChatId& chatId) const = 0;

    virtual void removeFriend(const ToxPk& friendPk) = 0;
    virtual void removeConference(const ConferenceId& conferenceId) = 0;

    virtual int chatroomCount() const = 0;
};
