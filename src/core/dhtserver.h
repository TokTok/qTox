/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "toxpk.h"

#include <QString>

#include <vector>

struct DhtServer
{
    bool statusUdp;
    bool statusTcp;
    QString ipv4;
    QString ipv6;
    QString maintainer;
    ToxPk publicKey;
    quint16 udpPort;
    std::vector<uint16_t> tcpPorts;

    bool operator==(const DhtServer& other) const;
    bool operator!=(const DhtServer& other) const;
};
