/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "flyoutoverlaywidget.h"

#include <QBitmap>
#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QTimer>

FlyoutOverlayWidget::FlyoutOverlayWidget(QWidget* parent)
    : QWidget(parent)
{
    setContentsMargins(0, 0, 0, 0);

    animation = new QPropertyAnimation(this, QByteArrayLiteral("flyoutPercent"), this);
    animation->setKeyValueAt(0, 0.0f);
    animation->setKeyValueAt(1, 1.0f);
    animation->setDuration(200);

    connect(animation, &QAbstractAnimation::finished, this, &FlyoutOverlayWidget::finishedAnimation);
    setFlyoutPercent(0);
    hide();
}

FlyoutOverlayWidget::~FlyoutOverlayWidget()
{
}

int FlyoutOverlayWidget::animationDuration() const
{
    return animation->duration();
}

void FlyoutOverlayWidget::setAnimationDuration(int timeMs)
{
    animation->setDuration(timeMs);
}

qreal FlyoutOverlayWidget::flyoutPercent() const
{
    return percent;
}

void FlyoutOverlayWidget::setFlyoutPercent(qreal progress)
{
    percent = progress;

    QSize self = size();
    setMask(QRegion(0, 0, self.width() * progress + 1, self.height()));
    move(startPos.x() + self.width() - self.width() * percent, startPos.y());
    setVisible(progress != 0);
}

bool FlyoutOverlayWidget::isShown() const
{
    return (percent == 1);
}

bool FlyoutOverlayWidget::isBeingAnimated() const
{
    return (animation->state() == QAbstractAnimation::Running);
}

bool FlyoutOverlayWidget::isBeingShown() const
{
    return (isBeingAnimated() && animation->direction() == QAbstractAnimation::Forward);
}

void FlyoutOverlayWidget::animateShow()
{
    if (percent == 1.0)
        return;

    if (animation->state() != QAbstractAnimation::Running)
        startPos = pos();

    startAnimation(true);
}

void FlyoutOverlayWidget::animateHide()
{
    if (animation->state() != QAbstractAnimation::Running)
        startPos = pos();

    startAnimation(false);
}

void FlyoutOverlayWidget::finishedAnimation()
{
    bool hide = (animation->direction() == QAbstractAnimation::Backward);

    // Delay it by a few frames to let the system catch up on rendering
    if (hide)
        QTimer::singleShot(50, this, SIGNAL(hidden()));
}

void FlyoutOverlayWidget::startAnimation(bool forward)
{
    setAttribute(Qt::WA_TransparentForMouseEvents, !forward);
    animation->setDirection(forward ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    animation->start();
    animation->setCurrentTime(animation->duration() * percent);
}
