/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "identicon.h"

#include <QColor>
#include <QCryptographicHash>
#include <QDebug>
#include <QImage>

#include <cassert>

namespace {
constexpr uint64_t COLOR_INT_MAX =
    (static_cast<uint64_t>(1) << (8 * Identicon::IDENTICON_COLOR_BYTES)) - 1;
}

/**
 * @brief Creates an Identicon, that visualizes a hash in graphical form.
 * @param data Data to visualize
 */
Identicon::Identicon(const QByteArray& data)
{
    static_assert(Identicon::COLORS == 2, "Only two colors are implemented");
    // hash with sha256
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    for (int colorIndex = 0; colorIndex < COLORS; ++colorIndex) {
        const QByteArray hashPart = hash.right(IDENTICON_COLOR_BYTES);
        hash.truncate(hash.length() - IDENTICON_COLOR_BYTES);

        const qreal hue = bytesToColor(hashPart);
        // change offset when COLORS != 2
        const qreal lig = static_cast<qreal>(colorIndex) / COLORS + 0.3;
        const qreal sat = 0.5;
        colors[colorIndex].setHslF(hue, sat, lig);
    }

    const auto* const hashBytes = reinterpret_cast<const uint8_t*>(hash.constData());
    // compute the block colors from the hash
    for (int row = 0; row < IDENTICON_ROWS; ++row) {
        for (int col = 0; col < ACTIVE_COLS; ++col) {
            const int hashIdx = row * ACTIVE_COLS + col;
            const uint8_t colorIndex = hashBytes[hashIdx] % COLORS;
            identiconColors[row][col] = colorIndex;
        }
    }
}

/**
 * @brief Converts a series of IDENTICON_COLOR_BYTES bytes to a value in the range 0.0..1.0
 * @param bytes Bytes to convert to a color
 * @return Value in the range of 0.0..1.0
 */
qreal Identicon::bytesToColor(QByteArray bytes)
{
    static_assert(IDENTICON_COLOR_BYTES <= 8, "IDENTICON_COLOR max value is 8");
    const auto* const bytesChr = reinterpret_cast<const uint8_t*>(bytes.constData());
    assert(bytes.length() == IDENTICON_COLOR_BYTES);

    // get foreground color
    uint64_t hue = bytesChr[0];

    // convert the last bytes to an uint
    for (int i = 1; i < IDENTICON_COLOR_BYTES; ++i) {
        hue = hue << 8;
        hue += bytesChr[i];
    }

    // normalize to 0.0 ... 1.0
    return static_cast<qreal>(hue) / static_cast<qreal>(COLOR_INT_MAX);
}

/**
 * @brief Prepares the color matrix for the identicon.
 * @return The identicon color matrix.
 */
Identicon::Matrix Identicon::toMatrix() const
{
    Matrix matrix{colors, {}};
    for (int row = 0; row < IDENTICON_ROWS; ++row) {
        for (int col = 0; col < IDENTICON_ROWS; ++col) {
            // mirror on vertical axis
            const int columnIdx = abs((col * 2 - (IDENTICON_ROWS - 1)) / 2);
            const int colorIdx = identiconColors[row][columnIdx];
            matrix.identicon[row][col] = colorIdx;
        }
    }
    return matrix;
}

QImage Identicon::Matrix::toImage(int scaleFactor) const
{
    if (scaleFactor < 1) {
        qDebug() << "Can't scale with values <1, clamping to 1";
        scaleFactor = 1;
    }

    scaleFactor *= IDENTICON_ROWS;

    QImage pixels(IDENTICON_ROWS, IDENTICON_ROWS, QImage::Format_RGB888);

    for (int row = 0; row < IDENTICON_ROWS; ++row) {
        for (int col = 0; col < IDENTICON_ROWS; ++col) {
            pixels.setPixel(col, row, colors[identicon[row][col]].rgb());
        }
    }

    // scale up without smoothing to make it look sharp
    return pixels.scaled(scaleFactor, scaleFactor, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

/**
 * @brief Writes the Identicon to a QImage.
 * @param scaleFactor the image will be a square with scaleFactor * IDENTICON_ROWS pixels,
 *                    must be >= 1
 * @return a QImage with the identicon.
 */
QImage Identicon::toImage(int scaleFactor) const
{
    return toMatrix().toImage(scaleFactor);
}

bool operator==(const Identicon::Matrix& lhs, const Identicon::Matrix& rhs)
{
    return lhs.colors == rhs.colors && lhs.identicon == rhs.identicon;
}

QDebug operator<<(QDebug debug, const Identicon::Matrix& matrix)
{
    debug << "Colors: " << matrix.colors[0] << ", " << matrix.colors[1] << "\n";
    for (int row = 0; row < Identicon::IDENTICON_ROWS; ++row) {
        for (int col = 0; col < Identicon::IDENTICON_ROWS; ++col) {
            debug << matrix.identicon[row][col];
        }
        debug << '\n';
    }
    return debug;
}

Identicon::Matrix Identicon::Matrix::parse(std::array<QColor, COLORS> colors, const QString& str)
{
    if (str.length() != IDENTICON_ROWS * IDENTICON_ROWS) {
        qWarning() << "Invalid string length, must be" << IDENTICON_ROWS * IDENTICON_ROWS
                   << "but is" << str.length();
        return {};
    }

    Data<int> matrix;
    for (int row = 0; row < IDENTICON_ROWS; ++row) {
        for (int col = 0; col < IDENTICON_ROWS; ++col) {
            const int value = str.at(row * IDENTICON_ROWS + col).digitValue();
            if (value < 0 || value >= COLORS) {
                qWarning() << "Invalid value" << value << "at" << row << ":" << col
                           << "must be between 0 and" << (COLORS - 1);
                return {};
            }
            matrix[row][col] = value;
        }
    }
    return {colors, matrix};
}
