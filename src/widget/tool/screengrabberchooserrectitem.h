/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QGraphicsItemGroup>

class ScreenGrabberChooserRectItem final : public QObject, public QGraphicsItemGroup
{
    Q_OBJECT
public:
    explicit ScreenGrabberChooserRectItem(QGraphicsScene* scene);
    ~ScreenGrabberChooserRectItem();

    QRectF boundingRect() const final;
    void beginResize(QPointF mousePos);

    QRect chosenRect() const;

    void showHandles();
    void hideHandles();

signals:

    void doubleClicked();
    void regionChosen(QRect rect);

protected:
    bool sceneEventFilter(QGraphicsItem* watched, QEvent* event) final;

private:
    enum State
    {
        None,
        Resizing,
        HandleResizing,
        Moving,
    };

    State state = None;
    int rectWidth = 0;
    int rectHeight = 0;
    QPointF startPos;

    void forwardMainRectEvent(QEvent* event);
    void forwardHandleEvent(QGraphicsItem* watched, QEvent* event);

    void mousePress(QGraphicsSceneMouseEvent* event);
    void mouseMove(QGraphicsSceneMouseEvent* event);
    void mouseRelease(QGraphicsSceneMouseEvent* event);
    void mouseDoubleClick(QGraphicsSceneMouseEvent* event);

    void mousePressHandle(int x, int y, QGraphicsSceneMouseEvent* event);
    void mouseMoveHandle(int x, int y, QGraphicsSceneMouseEvent* event);
    void mouseReleaseHandle(int x, int y, QGraphicsSceneMouseEvent* event);

    QPoint getHandleMultiplier(QGraphicsItem* handle);

    void updateHandlePositions();
    QGraphicsRectItem* createHandleItem(QGraphicsScene* scene);

    QGraphicsRectItem* mainRect;
    QGraphicsRectItem* topLeft;
    QGraphicsRectItem* topCenter;
    QGraphicsRectItem* topRight;
    QGraphicsRectItem* rightCenter;
    QGraphicsRectItem* bottomRight;
    QGraphicsRectItem* bottomCenter;
    QGraphicsRectItem* bottomLeft;
    QGraphicsRectItem* leftCenter;
};
