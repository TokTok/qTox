/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "screenshotgrabber.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScreen>
#include <QTimer>

#include "screengrabberchooserrectitem.h"
#include "screengrabberoverlayitem.h"
#include "toolboxgraphicsitem.h"
#include "src/widget/widget.h"

ScreenshotGrabber::ScreenshotGrabber()
    : QObject()
    , mKeysBlocked(false)
    , scene(nullptr)
    , mQToxVisible(true)
{
    window = new QGraphicsView(scene); // Top-level widget
    window->setWindowFlags(Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    window->setContentsMargins(0, 0, 0, 0);
    window->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    window->setFrameShape(QFrame::NoFrame);
    window->installEventFilter(this);
    pixRatio = QApplication::primaryScreen()->devicePixelRatio();

    setupScene();
}

void ScreenshotGrabber::reInit()
{
    window->resetCachedContent();
    setupScene();
    showGrabber();
    mKeysBlocked = false;
}

ScreenshotGrabber::~ScreenshotGrabber()
{
    delete scene;
    delete window;
}

bool ScreenshotGrabber::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
        return handleKeyPress(static_cast<QKeyEvent*>(event));

    return QObject::eventFilter(object, event);
}

void ScreenshotGrabber::showGrabber()
{
    screenGrab = grabScreen();
    screenGrabDisplay->setPixmap(screenGrab);
    window->show();
    window->setFocus();
    window->grabKeyboard();

    const QRect fullGrabbedRect = screenGrab.rect();
    const QRect rec = QApplication::primaryScreen()->virtualGeometry();

    window->setGeometry(rec);
    scene->setSceneRect(fullGrabbedRect);
    overlay->setRect(fullGrabbedRect);

    adjustTooltipPosition();
}

bool ScreenshotGrabber::handleKeyPress(QKeyEvent* event)
{
    if (mKeysBlocked)
        return false;

    if (event->key() == Qt::Key_Escape)
        reject();
    else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        acceptRegion();
    else if (event->key() == Qt::Key_Space) {
        mKeysBlocked = true;

        if (mQToxVisible)
            hideVisibleWindows();
        else
            restoreHiddenWindows();

        window->hide();
        QTimer::singleShot(350, this, &ScreenshotGrabber::reInit);
    } else
        return false;

    return true;
}

void ScreenshotGrabber::acceptRegion()
{
    QRect rect = chooserRect->chosenRect();
    if (rect.width() < 1 || rect.height() < 1)
        return;

    // Scale the accepted region from DIPs to actual pixels
    rect.setRect(rect.x() * pixRatio, rect.y() * pixRatio, rect.width() * pixRatio,
                 rect.height() * pixRatio);

    emit regionChosen(rect);
    qDebug() << "Screenshot accepted, chosen region" << rect;
    const QPixmap pixmap = screenGrab.copy(rect);
    restoreHiddenWindows();
    emit screenshotTaken(pixmap);

    deleteLater();
}

void ScreenshotGrabber::setupScene()
{
    delete scene;
    scene = new QGraphicsScene;
    window->setScene(scene);

    overlay = new ScreenGrabberOverlayItem(this);
    helperToolbox = new ToolBoxGraphicsItem;

    screenGrabDisplay = scene->addPixmap(screenGrab);
    helperTooltip = scene->addText(QString());

    scene->addItem(overlay);
    chooserRect = new ScreenGrabberChooserRectItem(scene);
    scene->addItem(helperToolbox);

    helperToolbox->addToGroup(helperTooltip);
    helperTooltip->setDefaultTextColor(Qt::black);
    useNothingSelectedTooltip();

    connect(chooserRect, &ScreenGrabberChooserRectItem::doubleClicked, this,
            &ScreenshotGrabber::acceptRegion);
    connect(chooserRect, &ScreenGrabberChooserRectItem::regionChosen, this,
            &ScreenshotGrabber::chooseHelperTooltipText);
    connect(chooserRect, &ScreenGrabberChooserRectItem::regionChosen, overlay,
            &ScreenGrabberOverlayItem::setChosenRect);
}

void ScreenshotGrabber::useNothingSelectedTooltip()
{
    helperTooltip->setHtml(
        tr("Click and drag to select a region. Press %1 to "
           "hide/show qTox window, or %2 to cancel.",
           "Help text shown when no region has been selected yet")
            .arg(QString("<b>%1</b>").arg(tr("Space", "[Space] key on the keyboard")),
                 QString("<b>%1</b>").arg(tr("Escape", "[Escape] key on the keyboard"))));
    adjustTooltipPosition();
}

void ScreenshotGrabber::useRegionSelectedTooltip()
{
    helperTooltip->setHtml(
        tr("Press %1 to send a screenshot of the selection, "
           "%2 to hide/show qTox window, or %3 to cancel.",
           "Help text shown when a region has been selected")
            .arg(QString("<b>%1</b>").arg(tr("Enter", "[Enter] key on the keyboard")),
                 QString("<b>%1</b>").arg(tr("Space", "[Space] key on the keyboard")),
                 QString("<b>%1</b>").arg(tr("Escape", "[Escape] key on the keyboard"))));
    adjustTooltipPosition();
}

void ScreenshotGrabber::chooseHelperTooltipText(QRect rect)
{
    if (rect.size().isNull())
        useNothingSelectedTooltip();
    else
        useRegionSelectedTooltip();
}

/**
 * @internal
 * @brief Align the tooltip centered at top of screen with the mouse cursor.
 */
void ScreenshotGrabber::adjustTooltipPosition()
{
    const QRect recGL = QGuiApplication::primaryScreen()->virtualGeometry();
    const auto rec = QGuiApplication::screenAt(QCursor::pos())->geometry();
    const QRectF ttRect = helperToolbox->childrenBoundingRect();
    const int x = qAbs(recGL.x()) + rec.x() + ((rec.width() - ttRect.width()) / 2);
    const int y = qAbs(recGL.y()) + rec.y();

    helperToolbox->setX(x);
    helperToolbox->setY(y);
}

void ScreenshotGrabber::reject()
{
    restoreHiddenWindows();
    deleteLater();
}

QPixmap ScreenshotGrabber::grabScreen() const
{
    QScreen* screen = QGuiApplication::primaryScreen();
    const QRect rec = screen->virtualGeometry();

    // Multiply by devicePixelRatio to get actual desktop size
    return screen->grabWindow(0, rec.x() * pixRatio, rec.y() * pixRatio, rec.width() * pixRatio,
                              rec.height() * pixRatio);
}

void ScreenshotGrabber::hideVisibleWindows()
{
    foreach (QWidget* w, qApp->topLevelWidgets()) {
        if (w != window && w->isVisible()) {
            mHiddenWindows << w;
            w->setVisible(false);
        }
    }

    mQToxVisible = false;
}

void ScreenshotGrabber::restoreHiddenWindows()
{
    foreach (QWidget* w, mHiddenWindows) {
        if (w != nullptr)
            w->setVisible(true);
    }

    mHiddenWindows.clear();
    mQToxVisible = true;
}

void ScreenshotGrabber::beginRectChooser(QGraphicsSceneMouseEvent* event)
{
    const QPointF pos = event->scenePos();
    chooserRect->setX(pos.x());
    chooserRect->setY(pos.y());
    chooserRect->beginResize(event->scenePos());
}
