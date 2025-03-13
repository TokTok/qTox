/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QMetaType>

enum class ConferenceType
{
    UNKNOWN,
    TEXT,
    AV,
};
Q_DECLARE_METATYPE(ConferenceType)

QDebug operator<<(QDebug debug, ConferenceType type);
