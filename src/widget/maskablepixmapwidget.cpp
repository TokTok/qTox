/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "maskablepixmapwidget.h"
#include <QPainter>
#include <QStyle>

/**
 * @var QPixmap* MaskablePixmapWidget::renderTarget
 * @brief pointer to dynamically call the constructor.
 */

MaskablePixmapWidget::MaskablePixmapWidget(QWidget* parent, QSize size, QString maskName_)
    : QLabel("", parent)
    , renderTarget(nullptr)
    , maskName(maskName_)
    , clickable(false)
{
    setSize(size);
}

MaskablePixmapWidget::~MaskablePixmapWidget()
{
    delete renderTarget;
}

void MaskablePixmapWidget::setClickable(bool clickable_)
{
    clickable = clickable_;

    if (clickable) {
        setCursor(Qt::PointingHandCursor);
    } else {
        unsetCursor();
    }
}

void MaskablePixmapWidget::setPixmap(const QPixmap& pmap)
{
    if (pmap.isNull()) {
        return;
    }

    unscaled = pmap;
    pixmap = pmap.scaled(width(), height(),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation);
    updatePixmap();
    update();
}

QPixmap MaskablePixmapWidget::getPixmap() const
{
    return *renderTarget;
}

void MaskablePixmapWidget::setSize(QSize size)
{
    setFixedSize(size);
    delete renderTarget;
    renderTarget = new QPixmap(size);

    QPixmap pmapMask = QPixmap(maskName);
    if (!pmapMask.isNull()) {
        mask = pmapMask.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    if (!unscaled.isNull()) {
        pixmap = unscaled.scaled(width(), height(),
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
        updatePixmap();
        update();
    }
}

void MaskablePixmapWidget::mousePressEvent(QMouseEvent* event)
{
    std::ignore = event;
    if (clickable) {
        emit clicked();
    }
}

void MaskablePixmapWidget::updatePixmap()
{
    renderTarget->fill(Qt::transparent);

    QPoint offset((width() - pixmap.size().width()) / 2,
                  (height() - pixmap.size().height()) / 2); // centering the pixmap

    QPainter painter(renderTarget);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(offset, pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    painter.drawPixmap(0, 0, mask);
    painter.end();
    QLabel::setPixmap(*renderTarget);
}
