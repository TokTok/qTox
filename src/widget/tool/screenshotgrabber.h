/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QObject>
#include <QPixmap>
#include <QPointer>

class QGraphicsSceneMouseEvent;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class QGraphicsScene;
class QGraphicsView;
class QKeyEvent;
class ScreenGrabberChooserRectItem;
class ScreenGrabberOverlayItem;
class ToolBoxGraphicsItem;

class ScreenshotGrabber : public QObject
{
    Q_OBJECT
public:
    ScreenshotGrabber();
    ~ScreenshotGrabber() override;

    bool eventFilter(QObject* object, QEvent* event) override;

    void showGrabber();

public slots:
    void acceptRegion();
    void reInit();

signals:
    void screenshotTaken(const QPixmap& pixmap);
    void regionChosen(QRect region);
    void rejected();

private:
    friend class ScreenGrabberOverlayItem;
    bool mKeysBlocked;

    void setupScene();

    void useNothingSelectedTooltip();
    void useRegionSelectedTooltip();
    void chooseHelperTooltipText(QRect rect);
    void adjustTooltipPosition();

    bool handleKeyPress(QKeyEvent* event);
    void reject();

    QPixmap grabScreen();

    void hideVisibleWindows();
    void restoreHiddenWindows();

    void beginRectChooser(QGraphicsSceneMouseEvent* event);

private:
    QPixmap screenGrab;
    QGraphicsScene* scene;
    QGraphicsView* window;
    QGraphicsPixmapItem* screenGrabDisplay;
    ScreenGrabberOverlayItem* overlay;
    ScreenGrabberChooserRectItem* chooserRect;
    ToolBoxGraphicsItem* helperToolbox;
    QGraphicsTextItem* helperTooltip;

    qreal pixRatio = 1.0;

    bool mQToxVisible;
    QVector<QPointer<QWidget>> mHiddenWindows;
};
