/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QObject>
#include <QPixmap>
#include <QTimer>

#include "../chatlinecontent.h"

class QVariantAnimation;

class Spinner : public ChatLineContent
{
    Q_OBJECT
public:
    Spinner(const QString& img, QSize size, qreal speed);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWidth(float width) override;
    qreal getAscent() const override;

private slots:
    void timeout();

private:
    static constexpr int framerate = 30; // 30Hz
    QSize size;
    QPixmap pmap;
    float rotSpeed;
    float curRot;
    QTimer timer;
    qreal alpha = 0.0;
    QVariantAnimation* blendAnimation;
};
