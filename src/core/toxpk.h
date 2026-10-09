/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/core/chatid.h"

#include <QByteArray>

#include <cstdint>
#include <tox/tox.h>

class ToxPk : public ChatId
{
public:
    ToxPk();
    explicit ToxPk(QByteArray rawId);
    explicit ToxPk(const uint8_t* rawId);
    explicit ToxPk(const QString& pk);
    int getSize() const override;
    std::unique_ptr<ChatId> clone() const override;
};
