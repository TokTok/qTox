/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QColor>
#include <QImage>

class Identicon
{
public:
    Identicon(const QByteArray& data);
    QImage toImage(int scaleFactor = 1);
    static qreal bytesToColor(QByteArray bytes);

public:
    static constexpr int IDENTICON_ROWS = 5;
    static constexpr int IDENTICON_COLOR_BYTES = 6;

private:
    static constexpr int COLORS = 2;
    static constexpr int ACTIVE_COLS = (IDENTICON_ROWS + 1) / 2;
    static constexpr int HASH_MIN_LEN = ACTIVE_COLS * IDENTICON_ROWS
                                      + COLORS * IDENTICON_COLOR_BYTES;

    uint8_t identiconColors[IDENTICON_ROWS][ACTIVE_COLS];
    QColor colors[COLORS];
};
