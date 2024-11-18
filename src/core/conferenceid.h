/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/chatid.h"
#include <QByteArray>
#include <cstdint>

class ConferenceId : public ChatId
{
public:
    static constexpr int size = 32;
    ConferenceId();
    explicit ConferenceId(const QByteArray& rawId);
    explicit ConferenceId(const uint8_t* rawId);
    int getSize() const override;
    std::unique_ptr<ChatId> clone() const override;
};
