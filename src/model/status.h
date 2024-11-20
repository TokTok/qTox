/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include <QPixmap>
#include <QString>

#pragma once

namespace Status {
// Status::Status is weird, but Status is a fitting name for both the namespace and enum class..
enum class Status
{
    Online = 0,
    Away,
    Busy,
    Offline,
    Blocked,
};

QString getIconPath(Status status, bool event = false);
QString getTitle(Status status);
QString getAssetSuffix(Status status);
bool isOnline(Status status);
} // namespace Status
