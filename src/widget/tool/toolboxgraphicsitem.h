/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QGraphicsItemGroup>
#include <QObject>
#include <QPropertyAnimation>

class ToolBoxGraphicsItem final : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    ToolBoxGraphicsItem();
    ~ToolBoxGraphicsItem();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) final;

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) final;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) final;

private:
    void startAnimation(QAbstractAnimation::Direction direction);

    QPropertyAnimation* opacityAnimation;
    qreal idleOpacity = 0.0;
    qreal activeOpacity = 1.0;
    int fadeTimeMs = 300;
};
