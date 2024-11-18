/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2021 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QDateTime>
#include <QObject>
#include <QString>

#include <array>

enum class SystemMessageType
{
    // DO NOT CHANGE ORDER
    // These values are saved directly to the DB and read back, changing the
    // order will break persistence!
    fileSendFailed = 0,
    userJoinedConference,
    userLeftConference,
    peerNameChanged,
    peerStateChange,
    titleChanged,
    cleared,
    unexpectedCallEnd,
    outgoingCall,
    incomingCall,
    callEnd,
    messageSendFailed,
    selfJoinedConference,
    selfLeftConference,
};

struct SystemMessage
{
    using Args = std::array<QString, 4>;
    SystemMessageType messageType;
    QDateTime timestamp;
    Args args;

    QString toString() const
    {
        switch (messageType) {
        case SystemMessageType::fileSendFailed:
            return QObject::tr("Failed to send file \"%1\"").arg(args[0]);
        case SystemMessageType::userJoinedConference:
            return QObject::tr("%1 has joined the conference").arg(args[0]);
        case SystemMessageType::userLeftConference:
            return QObject::tr("%1 has left the conference").arg(args[0]);
        case SystemMessageType::peerNameChanged:
            return QObject::tr("%1 is now known as %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::titleChanged:
            return QObject::tr("%1 has set the title to %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::cleared:
            return QObject::tr("Cleared");
        case SystemMessageType::unexpectedCallEnd:
            return QObject::tr("Call with %1 ended unexpectedly. %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::callEnd:
            return QObject::tr("Call with %1 ended. %2").arg(args[0]).arg(args[1]);
        case SystemMessageType::peerStateChange:
            return QObject::tr("%1 is now %2", "e.g. \"Dubslow is now online\"").arg(args[0]).arg(args[1]);
        case SystemMessageType::outgoingCall:
            return QObject::tr("Calling %1").arg(args[0]);
        case SystemMessageType::incomingCall:
            return QObject::tr("%1 calling").arg(args[0]);
        case SystemMessageType::messageSendFailed:
            return QObject::tr("Message failed to send");
        case SystemMessageType::selfJoinedConference:
            return QObject::tr("You have joined the conference");
        case SystemMessageType::selfLeftConference:
            return QObject::tr("You have left the conference");
        }
        return {};
    }
};
