/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include <QResizeEvent>
#include <QShowEvent>

#include "verticalonlyscroller.h"

VerticalOnlyScroller::VerticalOnlyScroller(QWidget* parent)
    : QScrollArea(parent)
{
}

void VerticalOnlyScroller::resizeEvent(QResizeEvent* event)
{
    QScrollArea::resizeEvent(event);
    if (widget() != nullptr)
        widget()->setMaximumWidth(event->size().width());
}

void VerticalOnlyScroller::showEvent(QShowEvent* event)
{
    QScrollArea::showEvent(event);
    if (widget() != nullptr)
        widget()->setMaximumWidth(size().width());
}
