/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "screengrabberoverlayitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>

#include "screenshotgrabber.h"

ScreenGrabberOverlayItem::ScreenGrabberOverlayItem(ScreenshotGrabber* grabber)
    : screenshotGrabber(grabber)
{
    QBrush overlayBrush(QColor(0x00, 0x00, 0x00, 0x70)); // Translucent black

    setCursor(QCursor(Qt::CrossCursor));
    setBrush(overlayBrush);
    setPen(QPen(Qt::NoPen));
}

ScreenGrabberOverlayItem::~ScreenGrabberOverlayItem() {}

void ScreenGrabberOverlayItem::setChosenRect(QRect rect)
{
    QRect oldRect = chosenRect;
    chosenRect = rect;
    update(oldRect.united(rect));
}

void ScreenGrabberOverlayItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        screenshotGrabber->beginRectChooser(event);
}

void ScreenGrabberOverlayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                     QWidget* widget)
{
    std::ignore = option;
    std::ignore = widget;
    painter->setBrush(brush());
    painter->setPen(pen());

    QRectF self = rect();
    qreal leftX = chosenRect.x();
    qreal rightX = chosenRect.x() + chosenRect.width();
    qreal topY = chosenRect.y();
    qreal bottomY = chosenRect.y() + chosenRect.height();

    painter->drawRect(0, 0, leftX, self.height());                      // Left of chosen
    painter->drawRect(rightX, 0, self.width() - rightX, self.height()); // Right of chosen
    painter->drawRect(leftX, 0, chosenRect.width(), topY);              // Top of chosen
    painter->drawRect(leftX, bottomY, chosenRect.width(),
                      self.height() - bottomY); // Bottom of chosen
}
