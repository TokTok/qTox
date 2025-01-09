/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QObject>
#include <QPixmap>

#include "../chatlinecontent.h"

class Broken : public ChatLineContent
{
    Q_OBJECT
public:
    Broken(const QString& img, QSize size_);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWidth(float width) override;
    qreal getAscent() const override;

private:
    QPixmap pmap;
    QSize size;
};
