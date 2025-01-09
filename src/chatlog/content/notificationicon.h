/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QLinearGradient>
#include <QPixmap>
#include <QTimer>

#include "../chatlinecontent.h"

class Settings;
class Style;

class NotificationIcon : public ChatLineContent
{
    Q_OBJECT
public:
    NotificationIcon(Settings& settings, Style& style, QSize size);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWidth(float width) override;
    qreal getAscent() const override;

private slots:
    void updateGradient();

private:
    static constexpr int framerate = 30; // 30Hz

    QSize size;
    QPixmap pmap;
    QLinearGradient grad;
    QTimer updateTimer;

    qreal dotWidth = 0.2;
    qreal alpha = 0.0;
};
