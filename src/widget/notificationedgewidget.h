/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>

class QLabel;
class Settings;
class Style;

class NotificationEdgeWidget final : public QWidget
{
    Q_OBJECT
public:
    enum Position : uint8_t
    {
        Top,
        Bottom
    };

    NotificationEdgeWidget(Position position, Settings& settings, Style& style,
        QWidget* parent = nullptr);
    void updateNotificationCount(int count);

signals:
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent* event) final;

private:
    QLabel* textLabel;
};
