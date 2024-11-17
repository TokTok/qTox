/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "friendmessagedispatcher.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"

FriendMessageDispatcher::FriendMessageDispatcher(Friend& f_, MessageProcessor processor_,
                                                 ICoreFriendMessageSender& messageSender_)
    : f(f_)
    , messageSender(messageSender_)
    , processor(processor_)
{
    connect(&f, &Friend::onlineOfflineChanged, this,
            &FriendMessageDispatcher::onFriendOnlineOfflineChanged);
}

/**
 * @see IMessageDispatcher::sendMessage
 */
std::pair<DispatchedMessageId, DispatchedMessageId>
FriendMessageDispatcher::sendMessage(bool isAction, const QString& content)
{
    const auto firstId = nextMessageId;
    auto lastId = nextMessageId;
    for (const auto& message : processor.processOutgoingMessage(isAction, content)) {
        auto messageId = nextMessageId++;
        lastId = messageId;

        auto onOfflineMsgComplete = getCompletionFn(messageId);
        sendProcessedMessage(message, onOfflineMsgComplete);

        emit messageSent(messageId, message);
    }
    return std::make_pair(firstId, lastId);
}

/**
 * @brief Handles received message from toxcore
 * @param[in] isAction True if action message
 * @param[in] content Unprocessed toxcore message
 */
void FriendMessageDispatcher::onMessageReceived(bool isAction, const QString& content)
{
    emit messageReceived(f.getPublicKey(), processor.processIncomingCoreMessage(isAction, content));
}

/**
 * @brief Handles received receipt from toxcore
 * @param[in] receipt receipt id
 */
void FriendMessageDispatcher::onReceiptReceived(ReceiptNum receipt)
{
    offlineMsgEngine.onReceiptReceived(receipt);
}

/**
 * @brief Handles status change for friend
 * @note Parameters just to fit slot api
 */
void FriendMessageDispatcher::onFriendOnlineOfflineChanged(const ToxPk& friendPk, bool isOnline)
{
    std::ignore = friendPk;
    if (isOnline) {
        auto messagesToResend = offlineMsgEngine.removeAllMessages();
        for (const auto& message : messagesToResend) {
            sendProcessedMessage(message.message, message.callback);
        }
    }
}

/**
 * @brief Clears all currently outgoing messages
 */
void FriendMessageDispatcher::clearOutgoingMessages()
{
    offlineMsgEngine.removeAllMessages();
}


void FriendMessageDispatcher::sendProcessedMessage(const Message& message,
                                                   OfflineMsgEngine::CompletionFn onOfflineMsgComplete)
{
    if (!Status::isOnline(f.getStatus())) {
        offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
        return;
    }

    sendCoreProcessedMessage(message, onOfflineMsgComplete);
}


void FriendMessageDispatcher::sendCoreProcessedMessage(const Message& message,
                                                       OfflineMsgEngine::CompletionFn onOfflineMsgComplete)
{
    auto receipt = ReceiptNum();

    const uint32_t friendId = f.getId();

    auto sendFn = message.isAction ? std::mem_fn(&ICoreFriendMessageSender::sendAction)
                                   : std::mem_fn(&ICoreFriendMessageSender::sendMessage);

    const auto messageSent = sendFn(messageSender, friendId, message.content, receipt);

    if (messageSent) {
        offlineMsgEngine.addSentCoreMessage(receipt, message, onOfflineMsgComplete);
    } else {
        offlineMsgEngine.addUnsentMessage(message, onOfflineMsgComplete);
    }
}

OfflineMsgEngine::CompletionFn FriendMessageDispatcher::getCompletionFn(DispatchedMessageId messageId)
{
    return [this, messageId](bool success) {
        if (success) {
            emit messageComplete(messageId);
        } else {
            // For now we know the only reason we can fail after giving to the
            // offline message engine is due to a reduced extension set
            emit messageBroken(messageId, BrokenMessageReason::unsupportedExtensions);
        }
    };
}
