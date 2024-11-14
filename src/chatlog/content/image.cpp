/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "image.h"
#include "../pixmapcache.h"

#include <QPainter>

Image::Image(QSize Size, const QString& filename)
    : size(Size)
{
    pmap = PixmapCache::getInstance().get(filename, size);
}

QRectF Image::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

qreal Image::getAscent() const
{
    return 0.0;
}

void Image::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setClipRect(boundingRect());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->translate(-size.width() / 2.0, -size.height() / 2.0);
    painter->drawPixmap(0, 0, pmap);

    std::ignore = option;
    std::ignore = widget;
}

void Image::setWidth(float width)
{
    std::ignore = width;
}
