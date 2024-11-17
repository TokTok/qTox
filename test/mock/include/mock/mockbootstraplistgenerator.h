/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/core/core.h"
#include "src/core/dhtserver.h"
#include "src/model/ibootstraplistgenerator.h"

#include <QList>

#include <mutex>

class MockBootstrapListGenerator : public IBootstrapListGenerator
{
public:
    MockBootstrapListGenerator() = default;
    ~MockBootstrapListGenerator() override;

    QList<DhtServer> getBootstrapNodes() const override
    {
        const std::lock_guard<std::mutex> lock(mutex);
        return bootstrapNodes;
    }

    void setBootstrapNodes(QList<DhtServer> list)
    {
        const std::lock_guard<std::mutex> lock(mutex);
        bootstrapNodes = list;
    }

    static QList<DhtServer> makeListFromSelf(const Core& core)
    {
        auto selfDhtId = core.getSelfDhtId();
        const quint16 selfDhtPort = core.getSelfUdpPort();

        DhtServer ret;
        ret.ipv4 = "localhost";
        ret.publicKey = ToxPk{selfDhtId};
        ret.statusTcp = false;
        ret.statusUdp = true;
        ret.udpPort = selfDhtPort;
        return {ret};
    }

private:
    QList<DhtServer> bootstrapNodes;
    mutable std::mutex mutex;
};
