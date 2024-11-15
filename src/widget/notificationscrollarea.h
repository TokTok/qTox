/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "tool/adjustingscrollarea.h"
#include <QHash>

class GenericChatroomWidget;
class NotificationEdgeWidget;
class Settings;
class Style;

class NotificationScrollArea final : public AdjustingScrollArea
{
public:
    explicit NotificationScrollArea(QWidget* parent = nullptr);

public slots:
    void trackWidget(Settings& settings, Style& style, GenericChatroomWidget* widget);
    void updateVisualTracking();
    void updateTracking(GenericChatroomWidget* widget);

protected:
    void resizeEvent(QResizeEvent* event) final;

private slots:
    void findNextWidget();
    void findPreviousWidget();

private:
    enum Visibility : uint8_t
    {
        Visible,
        Above,
        Below
    };
    Visibility widgetVisible(QWidget* widget) const;
    void recalculateTopEdge();
    void recalculateBottomEdge();

    QHash<GenericChatroomWidget*, Visibility> trackedWidgets;
    NotificationEdgeWidget* topEdge = nullptr;
    NotificationEdgeWidget* bottomEdge = nullptr;
    size_t referencesAbove = 0;
    size_t referencesBelow = 0;
};
