/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/chatlog/chatmessage.h"
#include "src/core/receiptnum.h"
#include "src/model/message.h"
#include "src/persistence/db/rawdatabase.h"

#include <QDateTime>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <chrono>

class OfflineMsgEngine : public QObject
{
    Q_OBJECT
public:
    using CompletionFn = std::function<void(bool)>;
    OfflineMsgEngine() = default;
    void addUnsentMessage(const Message& message, CompletionFn completionCallback);
    void addSentCoreMessage(ReceiptNum receipt, const Message& message,
                            CompletionFn completionCallback);

    struct RemovedMessage
    {
        Message message;
        CompletionFn callback;
    };
    std::vector<RemovedMessage> removeAllMessages();

public slots:
    void onReceiptReceived(ReceiptNum receipt);

private:
    struct OfflineMessage
    {
        Message message;
        std::chrono::time_point<std::chrono::steady_clock> authorshipTime;
        CompletionFn completionFn;
    };

    QRecursiveMutex mutex;

    template <class ReceiptT>
    class ReceiptResolver
    {
    public:
        void notifyMessageSent(ReceiptT receipt, const OfflineMessage& message)
        {
            auto receivedReceiptIt =
                std::find(receivedReceipts.begin(), receivedReceipts.end(), receipt);

            if (receivedReceiptIt != receivedReceipts.end()) {
                receivedReceipts.erase(receivedReceiptIt);
                message.completionFn(true);
                return;
            }

            unAckedMessages[receipt] = message;
        }

        void notifyReceiptReceived(ReceiptT receipt)
        {
            auto unackedMessageIt = unAckedMessages.find(receipt);
            if (unackedMessageIt != unAckedMessages.end()) {
                unackedMessageIt->second.completionFn(true);
                unAckedMessages.erase(unackedMessageIt);
                return;
            }

            receivedReceipts.push_back(receipt);
        }

        std::vector<OfflineMessage> clear()
        {
            auto ret = std::vector<OfflineMessage>();
            std::transform(std::make_move_iterator(unAckedMessages.begin()),
                           std::make_move_iterator(unAckedMessages.end()), std::back_inserter(ret),
                           [](const std::pair<ReceiptT, OfflineMessage>& pair) {
                               return std::move(pair.second);
                           });

            receivedReceipts.clear();
            unAckedMessages.clear();
            return ret;
        }

        std::vector<ReceiptT> receivedReceipts;
        std::map<ReceiptT, OfflineMessage> unAckedMessages;
    };

    ReceiptResolver<ReceiptNum> receiptResolver;
    std::vector<OfflineMessage> unsentMessages;
};
