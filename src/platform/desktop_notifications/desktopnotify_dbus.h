/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QObject>

#include <memory>

class DesktopNotifyDBus : public QObject
{
    Q_OBJECT

public:
    explicit DesktopNotifyDBus(QObject* parent);
    ~DesktopNotifyDBus();

    bool showMessage(const QString& title, const QString& message, const QPixmap& pixmap);

signals:
    void messageClicked();

private slots:
    void notificationActionInvoked(QString actionKey, QString actionValue);
    void notificationActionInvoked(uint actionKey, QString actionValue);

private:
    class Private;
    const std::unique_ptr<Private> d;
};
