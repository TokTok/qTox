/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#pragma once

class QPixmap;
class QByteArray;

class ImageLoader
{
public:
    virtual ~ImageLoader();

    virtual QPixmap loadImage(const QByteArray& data) = 0;
};
