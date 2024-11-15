/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>

class QPropertyAnimation;

class FlyoutOverlayWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal flyoutPercent READ flyoutPercent WRITE setFlyoutPercent)
public:
    explicit FlyoutOverlayWidget(QWidget* parent = nullptr);
    ~FlyoutOverlayWidget();

    int animationDuration() const;
    void setAnimationDuration(int timeMs);

    qreal flyoutPercent() const;
    void setFlyoutPercent(qreal progress);

    bool isShown() const;
    bool isBeingAnimated() const;
    bool isBeingShown() const;

    void animateShow();
    void animateHide();

signals:

    void hidden();

private:
    void finishedAnimation();
    void startAnimation(bool forward);

    QWidget* container;
    QPropertyAnimation* animation;
    qreal percent = 1.0;
    QPoint startPos;
};
