/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QPixmap>

namespace ExifTransform {
enum class Orientation
{
    /* name corresponds to where the 0 row and 0 column is in form row-column
     * i.e. entry 5 here means that the 0'th row corresponds to the left side of the scene and
     * the 0'th column corresponds to the top of the captured scene. This means that the image
     * needs to be mirrored and rotated to be displayed.
     */
    TopLeft,
    TopRight,
    BottomRight,
    BottomLeft,
    LeftTop,
    RightTop,
    RightBottom,
    LeftBottom,
};

Orientation getOrientation(QByteArray imageData);
QImage applyTransformation(QImage image, Orientation orientation);
} // namespace ExifTransform
