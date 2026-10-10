/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#include "conferencetype.h"

#include <QDebug>

QDebug operator<<(QDebug debug, ConferenceType type)
{
    switch (type) {
    case ConferenceType::UNKNOWN:
        debug << "UNKNOWN";
        break;
    case ConferenceType::TEXT:
        debug << "TEXT";
        break;
    case ConferenceType::AV:
        debug << "AV";
        break;
    }
    return debug;
}
