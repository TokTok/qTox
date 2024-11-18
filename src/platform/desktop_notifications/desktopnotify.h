/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/notificationdata.h"

#include <QObject>

#include <memory>
#include <unordered_set>

class Settings;

class DesktopNotify : public QObject
{
    Q_OBJECT
public:
    explicit DesktopNotify(Settings& settings);
    ~DesktopNotify();

public slots:
    void notifyMessage(const NotificationData& notificationData);

signals:
    void notificationClosed();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
    Settings& settings;
};
