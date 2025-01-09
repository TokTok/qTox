/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "receiptnum.h"

#include <QString>

#include <cstdint>

class ICoreFriendMessageSender
{
public:
    ICoreFriendMessageSender() = default;
    virtual ~ICoreFriendMessageSender();
    ICoreFriendMessageSender(const ICoreFriendMessageSender&) = default;
    ICoreFriendMessageSender& operator=(const ICoreFriendMessageSender&) = default;
    ICoreFriendMessageSender(ICoreFriendMessageSender&&) = default;
    ICoreFriendMessageSender& operator=(ICoreFriendMessageSender&&) = default;
    virtual bool sendAction(uint32_t friendId, const QString& action, ReceiptNum& receipt) = 0;
    virtual bool sendMessage(uint32_t friendId, const QString& message, ReceiptNum& receipt) = 0;
};
