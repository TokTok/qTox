/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "dhtserver.h"

/**
 * @brief   Compare equal operator
 * @param   other   the compared instance
 * @return  true, if equal; false otherwise
 */
bool DhtServer::operator==(const DhtServer& other) const
{
    return this == &other ||
        (statusUdp == other.statusUdp
        && statusTcp == other.statusTcp
        && ipv4 == other.ipv4
        && ipv6 == other.ipv6
        && maintainer == other.maintainer
        && publicKey == other.publicKey
        && udpPort == other.udpPort
        && tcpPorts == other.tcpPorts);
}

/**
 * @brief   Compare not equal operator
 * @param   other   the compared instance
 * @return  true, if not equal; false otherwise
 */
bool DhtServer::operator!=(const DhtServer& other) const
{
    return !(*this == other);
}
