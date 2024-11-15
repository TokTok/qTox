/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/icoreidhandler.h"

class MockCoreIdHandler : public ICoreIdHandler
{
public:
    MockCoreIdHandler() = default;
    virtual ~MockCoreIdHandler();
    MockCoreIdHandler(const MockCoreIdHandler&) = default;
    MockCoreIdHandler& operator=(const MockCoreIdHandler&) = default;
    MockCoreIdHandler(MockCoreIdHandler&&) = default;
    MockCoreIdHandler& operator=(MockCoreIdHandler&&) = default;

    ToxId getSelfId() const override
    {
        std::terminate();
        return ToxId();
    }

    ToxPk getSelfPublicKey() const override
    {
        static uint8_t id[ToxPk::size] = {0};
        return ToxPk(id);
    }

    QString getUsername() const override
    {
        return "me";
    }
};
