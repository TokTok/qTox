/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QPixmap>
#include <QString>

struct NotificationData
{
    QString title;
    QString message;
    QString category;
    QPixmap pixmap;
};
