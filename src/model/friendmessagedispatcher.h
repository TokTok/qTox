/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/icorefriendmessagesender.h"
#include "src/model/friend.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/message.h"
#include "src/persistence/offlinemsgengine.h"

#include <QObject>
#include <QString>

#include <cstdint>

class FriendMessageDispatcher : public IMessageDispatcher
{
    Q_OBJECT
public:
    FriendMessageDispatcher(Friend& f, MessageProcessor processor,
                            ICoreFriendMessageSender& messageSender);

    std::pair<DispatchedMessageId, DispatchedMessageId> sendMessage(bool isAction,
                                                                    const QString& content) override;

    void onMessageReceived(bool isAction, const QString& content);
    void onReceiptReceived(ReceiptNum receipt);
    void clearOutgoingMessages();
private slots:
    void onFriendOnlineOfflineChanged(const ToxPk& friendPk, bool isOnline);

private:
    void sendProcessedMessage(const Message& message,
                              OfflineMsgEngine::CompletionFn onOfflineMsgComplete);
    void sendCoreProcessedMessage(const Message& message,
                                  OfflineMsgEngine::CompletionFn onOfflineMsgComplete);
    OfflineMsgEngine::CompletionFn getCompletionFn(DispatchedMessageId messageId);

    Friend& f;
    DispatchedMessageId nextMessageId = DispatchedMessageId(0);

    ICoreFriendMessageSender& messageSender;
    OfflineMsgEngine offlineMsgEngine;
    MessageProcessor processor;
};
