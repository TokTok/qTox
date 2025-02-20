/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QColor>
#include <QImage>

class QDebug;

class Identicon
{
    // The following constants change the appearance of the identicon
    // they have been choosen by trying to make the output look nice.
public:
    /**
     * Specifies how many rows of blocks the identicon should have
     */
    static constexpr int IDENTICON_ROWS = 5;
    /**
     * Specifies how many bytes should define the foreground color
     * must be smaller than 8, else there'll be overflows
     */
    static constexpr int IDENTICON_COLOR_BYTES = 6;

private:
    /**
     * Number of colors to use for the identicon
     */
    static constexpr int COLORS = 2;
    /**
     * Width from the center to the outside, for 5 columns it's 3, 6 -> 3, 7 -> 4
     */
    static constexpr int ACTIVE_COLS = (IDENTICON_ROWS + 1) / 2;

public:
    struct Matrix
    {
        template <typename T>
        using Data = std::array<std::array<T, IDENTICON_ROWS>, IDENTICON_ROWS>;

        std::array<QColor, COLORS> colors;
        Data<int> identicon;

        QImage toImage(int scaleFactor) const;
        static Matrix parse(std::array<QColor, COLORS> colors, const QString& str);
    };

    explicit Identicon(const QByteArray& data);
    QImage toImage(int scaleFactor = 1) const;
    Matrix toMatrix() const;
    static qreal bytesToColor(QByteArray bytes);

private:
    std::array<std::array<uint8_t, ACTIVE_COLS>, IDENTICON_ROWS> identiconColors;
    std::array<QColor, COLORS> colors;
};

bool operator==(const Identicon::Matrix& lhs, const Identicon::Matrix& rhs);
QDebug operator<<(QDebug debug, const Identicon::Matrix& matrix);
