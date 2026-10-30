/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <tox/tox.h>

#include <QString>

#include <cstdint>

class ICoreGroupMessageSender
{
public:
    ICoreGroupMessageSender() = default;
    virtual ~ICoreGroupMessageSender();
    ICoreGroupMessageSender(const ICoreGroupMessageSender&) = default;
    ICoreGroupMessageSender& operator=(const ICoreGroupMessageSender&) = default;
    ICoreGroupMessageSender(ICoreGroupMessageSender&&) = default;
    ICoreGroupMessageSender& operator=(ICoreGroupMessageSender&&) = default;

    virtual void sendGroupAction(uint32_t groupNumber, const QString& message) = 0;
    virtual void sendGroupMessage(uint32_t groupNumber, const QString& message) = 0;
    virtual void sendGroupPrivateMessage(uint32_t groupNumber, uint32_t peerId,
                                         const QString& message, Tox_Message_Type type) = 0;
};
