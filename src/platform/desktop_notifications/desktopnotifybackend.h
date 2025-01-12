/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QObject>

#include <memory>

class DesktopNotifyBackend : public QObject
{
    Q_OBJECT

public:
    explicit DesktopNotifyBackend(QObject* parent);
    ~DesktopNotifyBackend() override;
    bool showMessage(const QString& title, const QString& message, const QString& category,
                     const QPixmap& pixmap);

signals:
    void messageClicked();

private slots:
    void notificationActionInvoked(QString actionKey, QString actionValue);
    void notificationActionInvoked(uint actionKey, QString actionValue);

private:
    struct Private;
    const std::unique_ptr<Private> d;
};
