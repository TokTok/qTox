/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "offlinemsgengine.h"
#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include <QCoreApplication>
#include <QMutexLocker>
#include <QTimer>
#include <chrono>

/**
 * @brief Notification that the message is now delivered.
 *
 * @param[in] receipt   Toxcore message ID which the receipt is for.
 */
void OfflineMsgEngine::onReceiptReceived(ReceiptNum receipt)
{
    const QMutexLocker<QRecursiveMutex> ml(&mutex);
    receiptResolver.notifyReceiptReceived(receipt);
}

/**
 * @brief Add a message which has been saved to history, but not sent yet to the peer.
 *
 * OfflineMsgEngine will send this message once the friend becomes online again, then track its
 * receipt, updating history and chat log once received.
 *
 * @param[in] messageID   database RowId of the message, used to eventually mark messages as received in history
 * @param[in] msg         chat message line in the chat log, used to eventually set the message's received timestamp
 */
void OfflineMsgEngine::addUnsentMessage(const Message& message, CompletionFn completionCallback)
{
    const QMutexLocker<QRecursiveMutex> ml(&mutex);
    unsentMessages.push_back(
        OfflineMessage{message, std::chrono::steady_clock::now(), completionCallback});
}

/**
 * @brief Add a message which has been saved to history, and which has been sent to the peer.
 *
 * OfflineMsgEngine will track this message's receipt. If the friend goes offline then comes back before the receipt
 * is received, OfflineMsgEngine will also resend the message, updating history and chat log once received.
 *
 * @param[in] receipt     the toxcore message ID, corresponding to expected receipt ID
 * @param[in] messageID   database RowId of the message, used to eventually mark messages as received in history
 * @param[in] msg         chat message line in the chat log, used to eventually set the message's received timestamp
 */
void OfflineMsgEngine::addSentCoreMessage(ReceiptNum receipt, const Message& message,
                                          CompletionFn completionCallback)
{
    const QMutexLocker<QRecursiveMutex> ml(&mutex);
    receiptResolver.notifyMessageSent(receipt, {message, std::chrono::steady_clock::now(),
                                                completionCallback});
}

/**
 * @brief Removes all messages which are being tracked.
 */
std::vector<OfflineMsgEngine::RemovedMessage> OfflineMsgEngine::removeAllMessages()
{
    const QMutexLocker<QRecursiveMutex> ml(&mutex);
    auto messages = receiptResolver.clear();

    messages.insert(messages.end(), std::make_move_iterator(unsentMessages.begin()),
                    std::make_move_iterator(unsentMessages.end()));

    unsentMessages.clear();

    std::sort(messages.begin(), messages.end(), [](const OfflineMessage& a, const OfflineMessage& b) {
        return a.authorshipTime < b.authorshipTime;
    });

    auto ret = std::vector<RemovedMessage>();
    ret.reserve(messages.size());

    std::transform(messages.begin(), messages.end(), std::back_inserter(ret),
                   [](const OfflineMessage& msg) {
                       return RemovedMessage{msg.message, msg.completionFn};
                   });

    return ret;
}
