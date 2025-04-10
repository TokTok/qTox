/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "screengrabberchooserrectitem.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

static constexpr int HandleSize = 10;
static constexpr qreal MinRectSize = 2;

ScreenGrabberChooserRectItem::ScreenGrabberChooserRectItem(QGraphicsScene* scene)
{
    scene->addItem(this);
    setCursor(QCursor(Qt::OpenHandCursor));

    mainRect = createHandleItem(scene);
    topLeft = createHandleItem(scene);
    topCenter = createHandleItem(scene);
    topRight = createHandleItem(scene);
    rightCenter = createHandleItem(scene);
    bottomRight = createHandleItem(scene);
    bottomCenter = createHandleItem(scene);
    bottomLeft = createHandleItem(scene);
    leftCenter = createHandleItem(scene);

    topLeft->setCursor(QCursor(Qt::SizeFDiagCursor));
    bottomRight->setCursor(QCursor(Qt::SizeFDiagCursor));
    topRight->setCursor(QCursor(Qt::SizeBDiagCursor));
    bottomLeft->setCursor(QCursor(Qt::SizeBDiagCursor));
    leftCenter->setCursor(QCursor(Qt::SizeHorCursor));
    rightCenter->setCursor(QCursor(Qt::SizeHorCursor));
    topCenter->setCursor(QCursor(Qt::SizeVerCursor));
    bottomCenter->setCursor(QCursor(Qt::SizeVerCursor));

    mainRect->setRect(QRect());
    hideHandles();
}

ScreenGrabberChooserRectItem::~ScreenGrabberChooserRectItem() = default;

QRectF ScreenGrabberChooserRectItem::boundingRect() const
{
    return {
        -HandleSize - 1,
        -HandleSize - 1,
        static_cast<qreal>(rectWidth + HandleSize + 1),
        static_cast<qreal>(rectHeight + HandleSize + 1),
    };
}

void ScreenGrabberChooserRectItem::beginResize(QPointF mousePos)
{
    rectWidth = rectHeight = 0;
    mainRect->setRect(QRect());
    state = Resizing;
    startPos = mousePos;

    setCursor(QCursor(Qt::CrossCursor));
    hideHandles();
    mainRect->grabMouse();
}

QRect ScreenGrabberChooserRectItem::chosenRect() const
{
    QRect rect(x(), y(), rectWidth, rectHeight);
    if (rectWidth < 0) {
        rect.setX(rect.x() + rectWidth);
        rect.setWidth(-rectWidth);
    }

    if (rectHeight < 0) {
        rect.setY(rect.y() + rectHeight);
        rect.setHeight(-rectHeight);
    }

    return rect;
}

void ScreenGrabberChooserRectItem::showHandles()
{
    topLeft->show();
    topCenter->show();
    topRight->show();
    rightCenter->show();
    bottomRight->show();
    bottomCenter->show();
    bottomLeft->show();
    leftCenter->show();
}

void ScreenGrabberChooserRectItem::hideHandles()
{
    topLeft->hide();
    topCenter->hide();
    topRight->hide();
    rightCenter->hide();
    bottomRight->hide();
    bottomCenter->hide();
    bottomLeft->hide();
    leftCenter->hide();
}

void ScreenGrabberChooserRectItem::mousePress(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        state = Moving;
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
}

void ScreenGrabberChooserRectItem::mouseMove(QGraphicsSceneMouseEvent* event)
{
    if (state == Moving) {
        const QPointF delta = event->scenePos() - event->lastScenePos();
        moveBy(delta.x(), delta.y());
    } else if (state == Resizing) {
        prepareGeometryChange();
        const QPointF size = event->scenePos() - scenePos();
        mainRect->setRect(0, 0, size.x(), size.y());
        rectWidth = size.x();
        rectHeight = size.y();

        updateHandlePositions();
    } else {
        return;
    }

    emit regionChosen(chosenRect());
}

void ScreenGrabberChooserRectItem::mouseRelease(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setCursor(QCursor(Qt::OpenHandCursor));

        const QPointF delta = (event->scenePos() - startPos);
        if (qAbs(delta.x()) < MinRectSize || qAbs(delta.y()) < MinRectSize) {
            rectWidth = rectHeight = 0;
            mainRect->setRect(QRect());
        } else {
            const QRect normalized = chosenRect();

            rectWidth = normalized.width();
            rectHeight = normalized.height();
            setPos(normalized.x(), normalized.y());
            mainRect->setRect(0, 0, rectWidth, rectHeight);

            updateHandlePositions();
            showHandles();
        }

        emit regionChosen(chosenRect());
        state = None;
        mainRect->ungrabMouse();
    }
}

void ScreenGrabberChooserRectItem::mouseDoubleClick(QGraphicsSceneMouseEvent* event)
{
    std::ignore = event;
    emit doubleClicked();
}

void ScreenGrabberChooserRectItem::mousePressHandle(int x, int y, QGraphicsSceneMouseEvent* event)
{
    std::ignore = x;
    std::ignore = y;

    if (event->button() == Qt::LeftButton)
        state = HandleResizing;
}

void ScreenGrabberChooserRectItem::mouseMoveHandle(int x, int y, QGraphicsSceneMouseEvent* event)
{
    if (state != HandleResizing)
        return;

    QPointF delta = event->scenePos() - event->lastScenePos();
    delta.rx() *= qAbs(x);
    delta.ry() *= qAbs(y);

    // We increase if the multiplier and the delta have the same sign
    const bool increaseX = ((x < 0) == (delta.x() < 0));
    const bool increaseY = ((y < 0) == (delta.y() < 0));

    if ((delta.x() < 0 && increaseX) || (delta.x() >= 0 && !increaseX)) {
        moveBy(delta.x(), 0);
        delta.rx() *= -1;
    }

    if ((delta.y() < 0 && increaseY) || (delta.y() >= 0 && !increaseY)) {
        moveBy(0, delta.y());
        delta.ry() *= -1;
    }

    //
    rectWidth += delta.x();
    rectHeight += delta.y();
    mainRect->setRect(0, 0, rectWidth, rectHeight);
    updateHandlePositions();
    emit regionChosen(chosenRect());
}

void ScreenGrabberChooserRectItem::mouseReleaseHandle(int x, int y, QGraphicsSceneMouseEvent* event)
{
    std::ignore = x;
    std::ignore = y;

    if (event->button() == Qt::LeftButton)
        state = None;
}

QPoint ScreenGrabberChooserRectItem::getHandleMultiplier(QGraphicsItem* handle)
{
    if (handle == topLeft)
        return {-1, -1};

    if (handle == topCenter)
        return {0, -1};

    if (handle == topRight)
        return {1, -1};

    if (handle == rightCenter)
        return {1, 0};

    if (handle == bottomRight)
        return {1, 1};

    if (handle == bottomCenter)
        return {0, 1};

    if (handle == bottomLeft)
        return {-1, 1};

    if (handle == leftCenter)
        return {-1, 0};

    return {};
}

void ScreenGrabberChooserRectItem::updateHandlePositions()
{
    topLeft->setPos(-HandleSize, -HandleSize);
    topCenter->setPos((rectWidth - HandleSize) / 2, -HandleSize);
    topRight->setPos(rectWidth, -HandleSize);
    rightCenter->setPos(rectWidth, (rectHeight - HandleSize) / 2);
    bottomRight->setPos(rectWidth, rectHeight);
    bottomCenter->setPos((rectWidth - HandleSize) / 2, rectHeight);
    bottomLeft->setPos(-HandleSize, rectHeight);
    leftCenter->setPos(-HandleSize, (rectHeight - HandleSize) / 2);
}

QGraphicsRectItem* ScreenGrabberChooserRectItem::createHandleItem(QGraphicsScene* scene)
{
    auto* handle = new QGraphicsRectItem(0, 0, HandleSize, HandleSize);
    handle->setPen(QPen(Qt::blue));
    handle->setBrush(Qt::NoBrush);

    scene->addItem(handle);
    addToGroup(handle);

    handle->installSceneEventFilter(this);
    return handle;
}

bool ScreenGrabberChooserRectItem::sceneEventFilter(QGraphicsItem* watched, QEvent* event)
{
    if (watched == mainRect)
        forwardMainRectEvent(event);
    else
        forwardHandleEvent(watched, event);

    return true;
}

void ScreenGrabberChooserRectItem::forwardMainRectEvent(QEvent* event)
{
    auto* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        return mousePress(mouseEvent);
    case QEvent::GraphicsSceneMouseMove:
        return mouseMove(mouseEvent);
    case QEvent::GraphicsSceneMouseRelease:
        return mouseRelease(mouseEvent);
    case QEvent::GraphicsSceneMouseDoubleClick:
        return mouseDoubleClick(mouseEvent);
    default:
        return;
    }
}

void ScreenGrabberChooserRectItem::forwardHandleEvent(QGraphicsItem* watched, QEvent* event)
{
    auto* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    const QPoint multiplier = getHandleMultiplier(watched);

    if (multiplier.isNull())
        return;

    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        return mousePressHandle(multiplier.x(), multiplier.y(), mouseEvent);
    case QEvent::GraphicsSceneMouseMove:
        return mouseMoveHandle(multiplier.x(), multiplier.y(), mouseEvent);
    case QEvent::GraphicsSceneMouseRelease:
        return mouseReleaseHandle(multiplier.x(), multiplier.y(), mouseEvent);
    default:
        return;
    }
}
