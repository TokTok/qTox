/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "exiftransform.h"

#include <libexif/exif-loader.h>

#include <QDebug>

namespace ExifTransform {
Orientation getOrientation(QByteArray imageData)
{
    auto data = imageData.constData();
    auto size = imageData.size();

    ExifData* exifData = exif_data_new_from_data(reinterpret_cast<const unsigned char*>(data), size);

    if (!exifData) {
        return Orientation::TopLeft;
    }

    const ExifByteOrder byteOrder = exif_data_get_byte_order(exifData);
    const ExifEntry* const exifEntry = exif_data_get_entry(exifData, EXIF_TAG_ORIENTATION);

    if (!exifEntry) {
        exif_data_free(exifData);
        return Orientation::TopLeft;
    }

    const int orientation = exif_get_short(exifEntry->data, byteOrder);
    exif_data_free(exifData);

    switch (orientation) {
    case 1:
        return Orientation::TopLeft;
    case 2:
        return Orientation::TopRight;
    case 3:
        return Orientation::BottomRight;
    case 4:
        return Orientation::BottomLeft;
    case 5:
        return Orientation::LeftTop;
    case 6:
        return Orientation::RightTop;
    case 7:
        return Orientation::RightBottom;
    case 8:
        return Orientation::LeftBottom;
    default:
        qWarning() << "Invalid exif orientation";
        return Orientation::TopLeft;
    }
}

QImage applyTransformation(QImage image, Orientation orientation)
{
    QTransform exifTransform;
    switch (orientation) {
    case Orientation::TopLeft:
        break;
    case Orientation::TopRight:
        image = image.mirrored(1, 0);
        break;
    case Orientation::BottomRight:
        exifTransform.rotate(180);
        break;
    case Orientation::BottomLeft:
        image = image.mirrored(0, 1);
        break;
    case Orientation::LeftTop:
        exifTransform.rotate(-90);
        image = image.mirrored(1, 0);
        break;
    case Orientation::RightTop:
        exifTransform.rotate(90);
        break;
    case Orientation::RightBottom:
        exifTransform.rotate(90);
        image = image.mirrored(1, 0);
        break;
    case Orientation::LeftBottom:
        exifTransform.rotate(-90);
        break;
    }
    image = image.transformed(exifTransform);
    return image;
}
} // namespace ExifTransform
