/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "toolboxgraphicsitem.h"

#include <QPainter>

ToolBoxGraphicsItem::ToolBoxGraphicsItem()
{
    opacityAnimation = new QPropertyAnimation(this, QByteArrayLiteral("opacity"), this);

    opacityAnimation->setKeyValueAt(0, idleOpacity);
    opacityAnimation->setKeyValueAt(1, activeOpacity);
    opacityAnimation->setDuration(fadeTimeMs);

    setOpacity(activeOpacity);
}

ToolBoxGraphicsItem::~ToolBoxGraphicsItem() {}

void ToolBoxGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    startAnimation(QAbstractAnimation::Backward);
    QGraphicsItemGroup::hoverEnterEvent(event);
}

void ToolBoxGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    startAnimation(QAbstractAnimation::Forward);
    QGraphicsItemGroup::hoverLeaveEvent(event);
}

void ToolBoxGraphicsItem::startAnimation(QAbstractAnimation::Direction direction)
{
    opacityAnimation->setDirection(direction);
    opacityAnimation->start();
}

void ToolBoxGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                QWidget* widget)
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(0xFF, 0xE2, 0x82)));
    painter->drawRect(childrenBoundingRect());
    painter->restore();

    QGraphicsItemGroup::paint(painter, option, widget);
}
