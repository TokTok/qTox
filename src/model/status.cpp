/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include <src/model/status.h>

#include <QString>
#include <QPixmap>
#include <QDebug>
#include <QObject>

#include <cassert>

namespace Status
{
    QString getTitle(Status status)
    {
        switch (status) {
        case Status::Online:
            return QObject::tr("online", "contact status");
        case Status::Away:
            return QObject::tr("away", "contact status");
        case Status::Busy:
            return QObject::tr("busy", "contact status");
        case Status::Offline:
            return QObject::tr("offline", "contact status");
        case Status::Blocked:
            return QObject::tr("blocked", "contact status");
        case Status::Negotiating:
            return QObject::tr("negotitating", "contact status");
        }

        assert(false);
        return QStringLiteral("");
    }

    QString getAssetSuffix(Status status)
    {
        switch (status) {
        case Status::Online:
            return "online";
        case Status::Away:
            return "away";
        case Status::Busy:
            return "busy";
        case Status::Offline:
            return "offline";
        case Status::Blocked:
            return "blocked";
        case Status::Negotiating:
            return "negotiating";
        }
        assert(false);
        return QStringLiteral("");
    }

    QString getIconPath(Status status, bool event)
    {
        const QString eventSuffix = event ? QStringLiteral("_notification") : QString();
        const QString statusSuffix = getAssetSuffix(status);
        if (status == Status::Blocked) {
            return ":/img/status/" + statusSuffix + ".svg";
        } else {
            return ":/img/status/" + statusSuffix + eventSuffix + ".svg";
        }
    }

    bool isOnline(Status status)
    {
        return status != Status::Offline
            && status != Status::Blocked
            // We don't want to treat a friend as online unless we know their feature set
            && status != Status::Negotiating;
    }
} // namespace Status
