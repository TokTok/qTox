/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "notificationicon.h"
#include "src/widget/style.h"
#include "../pixmapcache.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QTimer>

NotificationIcon::NotificationIcon(Settings& settings, Style& style, QSize Size)
    : size(Size)
{
    pmap = PixmapCache::getInstance().get(style.getImagePath("chatArea/typing.svg", settings), size);

    // Timer for the animation, if the Widget is not redrawn, no paint events will
    // arrive and the timer will not be restarted, so this stops automatically
    updateTimer.setInterval(1000 / framerate);
    updateTimer.setSingleShot(true);

    connect(&updateTimer, &QTimer::timeout, this, &NotificationIcon::updateGradient);
}

QRectF NotificationIcon::boundingRect() const
{
    return {{-size.width() / 2.0, -size.height() / 2.0}, size};
}

void NotificationIcon::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setClipRect(boundingRect());

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->translate(-size.width() / 2.0, -size.height() / 2.0);

    painter->fillRect(QRect(0, 0, size.width(), size.height()), grad);
    painter->drawPixmap(0, 0, size.width(), size.height(), pmap);

    if (!updateTimer.isActive()) {
        updateTimer.start();
    }

    std::ignore = option;
    std::ignore = widget;
}

void NotificationIcon::setWidth(float width)
{
    std::ignore = width;
}

qreal NotificationIcon::getAscent() const
{
    return 3.0;
}

void NotificationIcon::updateGradient()
{
    // Update for next frame
    alpha += 0.01;

    if (alpha + dotWidth >= 1.0) {
        alpha = 0.0;
    }

    grad = QLinearGradient(QPointF(-0.5 * size.width(), 0), QPointF(3.0 / 2.0 * size.width(), 0));
    grad.setColorAt(0, Qt::lightGray);
    grad.setColorAt(qMax(0.0, alpha - dotWidth), Qt::lightGray);
    grad.setColorAt(alpha, Qt::black);
    grad.setColorAt(qMin(1.0, alpha + dotWidth), Qt::lightGray);
    grad.setColorAt(1, Qt::lightGray);

    if ((scene() != nullptr) && isVisible()) {
        scene()->invalidate(sceneBoundingRect());
    }
}
