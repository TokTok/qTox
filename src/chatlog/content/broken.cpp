/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "broken.h"
#include "src/chatlog/pixmapcache.h"
#include <QPainter>

class QStyleOptionGraphicsItem;

Broken::Broken(const QString& img, QSize size_)
    : pmap{PixmapCache::getInstance().get(img, size_)}
    , size{size_}
{
}

QRectF Broken::boundingRect() const
{
    return QRectF(QPointF(-size.width() / 2.0, -size.height() / 2.0), size);
}

void Broken::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget)
{
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(0, 0, pmap);

    std::ignore = option;
    std::ignore = widget;

}

void Broken::setWidth(float width)
{
    std::ignore = width;
}

qreal Broken::getAscent() const
{
    return 0.0;
}
