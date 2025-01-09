/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QPixmap>

#include "../chatlinecontent.h"

class Image : public ChatLineContent
{
public:
    Image(QSize size, const QString& filename);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWidth(float width) override;
    qreal getAscent() const override;

private:
    QSize size;
    QPixmap pmap;
};
