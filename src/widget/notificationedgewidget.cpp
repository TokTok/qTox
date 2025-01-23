/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "notificationedgewidget.h"

#include "style.h"

#include "src/persistence/settings.h"

#include <QBoxLayout>
#include <QDebug>
#include <QLabel>

NotificationEdgeWidget::NotificationEdgeWidget(Position position, Settings& settings, Style& style,
                                               QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground); // Show background.
    setStyleSheet(style.getStylesheet("notificationEdge/notificationEdge.qss", settings));
    auto* layout = new QHBoxLayout(this);
    layout->addStretch();

    textLabel = new QLabel(this);
    textLabel->setMinimumHeight(textLabel->sizeHint().height()); // Prevent cut-off text.
    layout->addWidget(textLabel);

    auto* arrowLabel = new QLabel(this);

    if (position == Top)
        arrowLabel->setPixmap(QPixmap(style.getImagePath("chatArea/scrollBarUpArrow.svg", settings)));
    else
        arrowLabel->setPixmap(QPixmap(style.getImagePath("chatArea/scrollBarDownArrow.svg", settings)));

    layout->addWidget(arrowLabel);
    layout->addStretch();

    setCursor(Qt::PointingHandCursor);
}

void NotificationEdgeWidget::updateNotificationCount(int count)
{
    textLabel->setText(tr("Unread message(s)", "", count));
}

void NotificationEdgeWidget::mouseReleaseEvent(QMouseEvent* event)
{
    emit clicked();
    QWidget::mousePressEvent(event);
}
