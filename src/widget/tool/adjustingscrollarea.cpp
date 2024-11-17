/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "adjustingscrollarea.h"

#include <QDebug>
#include <QEvent>
#include <QLayout>
#include <QScrollBar>

AdjustingScrollArea::AdjustingScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
}

void AdjustingScrollArea::resizeEvent(QResizeEvent* ev)
{
    const int scrollBarWidth =
        verticalScrollBar()->isVisible() ? verticalScrollBar()->sizeHint().width() : 0;

    if (layoutDirection() == Qt::RightToLeft)
        setViewportMargins(-scrollBarWidth, 0, 0, 0);

    updateGeometry();
    QScrollArea::resizeEvent(ev);
}

QSize AdjustingScrollArea::sizeHint() const
{
    if (widget()) {
        const int scrollbarWidth = verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
        return widget()->sizeHint() + QSize(scrollbarWidth, 0);
    }

    return QScrollArea::sizeHint();
}
