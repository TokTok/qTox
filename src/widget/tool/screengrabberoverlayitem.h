/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QGraphicsRectItem>

class ScreenshotGrabber;

class ScreenGrabberOverlayItem final : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    explicit ScreenGrabberOverlayItem(ScreenshotGrabber* grabber);
    ~ScreenGrabberOverlayItem();

    void setChosenRect(QRect rect);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) final;

private:
    ScreenshotGrabber* screenshotGrabber;

    QRect chosenRect;
};
