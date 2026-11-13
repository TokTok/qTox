/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "groupmessagedispatcher.h"

#include "src/persistence/iconferencesettings.h"

#include <QtCore>

GroupMessageDispatcher::GroupMessageDispatcher(Group& g_, MessageProcessor processor_,
                                               ICoreIdHandler& idHandler_,
                                               ICoreGroupMessageSender& messageSender_,
                                               IConferenceSettings& settings_)
    : group(g_)
    , processor(processor_)
    , idHandler(idHandler_)
    , messageSender(messageSender_)
    , settings(settings_)
{
    processor.enableMentions();
}

std::pair<DispatchedMessageId, DispatchedMessageId>
GroupMessageDispatcher::sendMessage(bool isAction, const QString& content)
{
    const auto firstMessageId = nextMessageId;
    auto lastMessageId = firstMessageId;

    for (const auto& message : processor.processOutgoingMessage(isAction, content)) {
        auto messageId = nextMessageId++;
        lastMessageId = messageId;
        if (message.isAction) {
            messageSender.sendGroupAction(group.getId(), message.content);
        } else {
            messageSender.sendGroupMessage(group.getId(), message.content);
        }

        // Emit both signals since we do not have receipts for groups
        emit messageSent(messageId, message);
        emit messageComplete(messageId);
    }

    return std::make_pair(firstMessageId, lastMessageId);
}

std::pair<DispatchedMessageId, DispatchedMessageId>
GroupMessageDispatcher::sendPrivateMessage(const ToxPk& recipient, bool isAction, const QString& content)
{
    const auto firstMessageId = nextMessageId;
    auto lastMessageId = firstMessageId;
    const QString recipientName = group.getDisplayedName(recipient);

    for (const auto& message : processor.processOutgoingMessage(isAction, content)) {
        auto messageId = nextMessageId++;
        lastMessageId = messageId;
        if (message.isAction) {
            messageSender.sendGroupPrivateAction(group.getId(), recipient, message.content);
        } else {
            messageSender.sendGroupPrivateMessage(group.getId(), recipient, message.content);
        }

        Message messageWithRecipient = message;
        messageWithRecipient.recipient = recipient;
        messageWithRecipient.recipientName = recipientName;
        emit messageSent(messageId, messageWithRecipient);
        emit messageComplete(messageId);
    }

    return std::make_pair(firstMessageId, lastMessageId);
}

/**
 * @brief Processes and dispatches received message from toxcore
 * @param[in] sender
 * @param[in] isAction True if is action
 * @param[in] content Message content
 */
void GroupMessageDispatcher::onMessageReceived(const ToxPk& sender, bool isAction, const QString& content)
{
    const bool isSelf = sender == group.getSelfPeerPk();

    if (isSelf) {
        return;
    }

    if (settings.getBlockList().contains(sender.toString())) {
        qDebug() << "onGroupMessageReceived: Filtered:" << sender.toString();
        return;
    }

    emit messageReceived(sender, processor.processIncomingCoreMessage(isAction, content));
}

void GroupMessageDispatcher::onPrivateMessageReceived(const ToxPk& sender, bool isAction,
                                                      const QString& content)
{
    const bool isSelf = sender == group.getSelfPeerPk();

    if (isSelf) {
        return;
    }

    if (settings.getBlockList().contains(sender.toString())) {
        qDebug() << "onGroupPrivateMessageReceived: Filtered:" << sender.toString();
        return;
    }

    Message message = processor.processIncomingCoreMessage(isAction, content);
    message.recipient = group.getSelfPeerPk();
    message.recipientName = idHandler.getUsername();
    emit messageReceived(sender, message);
}
