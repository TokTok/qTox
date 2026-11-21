/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/core/chatid.h"

#include <QByteArray>

#include <cstdint>

class GroupId : public ChatId
{
public:
    static constexpr int size = 32;
    GroupId();
    explicit GroupId(const QByteArray& rawId);
    explicit GroupId(const uint8_t* rawId);
    int getSize() const override;
    std::unique_ptr<ChatId> clone() const override;
};
