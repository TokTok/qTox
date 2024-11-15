/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2021 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "util/display.h"

#include <cmath>

QString getHumanReadableSize(uint64_t size)
{
    static const char* suffix[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    int exp = 0;

    if (size > 0) {
        exp = std::min(static_cast<int>(log(size) / log(1024)), static_cast<int>(sizeof(suffix) / sizeof(suffix[0]) - 1));
    }

    return QString().setNum(size / pow(1024, exp), 'f', exp > 1 ? 2 : 0).append(QString::fromUtf8(suffix[exp]));
}
