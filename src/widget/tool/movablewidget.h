/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>

class MovableWidget : public QWidget
{
public:
    explicit MovableWidget(QWidget* parent);
    void resetBoundary(QRect newBoundary);
    void setBoundary(QRect newBoundary);
    float getRatio() const;
    void setRatio(float r);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);

private:
    void checkBoundary(QPoint& point) const;
    void checkBoundaryLeft(int& x) const;

    typedef uint8_t Modes;

    enum Mode : Modes
    {
        Moving = 0x01,
        ResizeLeft = 0x02,
        ResizeRight = 0x04,
        ResizeUp = 0x08,
        ResizeDown = 0x10,
        Resize = ResizeLeft | ResizeRight | ResizeUp | ResizeDown
    };

    Modes mode = 0;
    QPoint lastPoint;
    QRect boundaryRect;
    QSizeF actualSize;
    QPointF actualPos;
    float ratio;
};
