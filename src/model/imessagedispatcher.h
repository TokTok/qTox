/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/brokenmessagereason.h"
#include "src/model/friend.h"
#include "src/model/message.h"
#include "util/strongtype.h"

#include <QObject>
#include <QString>

#include <cstdint>

using DispatchedMessageId = NamedType<size_t, struct SentMessageIdTag, Orderable, Incrementable>;
Q_DECLARE_METATYPE(DispatchedMessageId)

class IMessageDispatcher : public QObject
{
    Q_OBJECT
public:
    virtual ~IMessageDispatcher();

    /**
     * @brief Sends message to associated chat
     * @param[in] isAction True if is action message
     * @param[in] content Message content
     * @return Pair of first and last dispatched message IDs
     */
    virtual std::pair<DispatchedMessageId, DispatchedMessageId>
    sendMessage(bool isAction, const QString& content) = 0;

signals:
    /**
     * @brief Emitted when a message is received and processed
     */
    void messageReceived(const ToxPk& sender, const Message& message);

    /**
     * @brief Emitted when a message is processed and sent
     * @param id message id for completion
     * @param message sent message
     */
    void messageSent(DispatchedMessageId id, const Message& message);

    /**
     * @brief Emitted when a receiver report is received from the associated chat
     * @param id Id of message that is completed
     */
    void messageComplete(DispatchedMessageId id);

    void messageBroken(DispatchedMessageId id, BrokenMessageReason reason);
};
