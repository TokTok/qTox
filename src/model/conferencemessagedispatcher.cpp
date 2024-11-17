/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "conferencemessagedispatcher.h"
#include "src/persistence/iconferencesettings.h"

#include <QtCore>

ConferenceMessageDispatcher::ConferenceMessageDispatcher(Conference& g_, MessageProcessor processor_,
                                                         ICoreIdHandler& idHandler_,
                                                         ICoreConferenceMessageSender& messageSender_,
                                                         const IConferenceSettings& conferenceSettings_)
    : conference(g_)
    , processor(processor_)
    , idHandler(idHandler_)
    , messageSender(messageSender_)
    , conferenceSettings(conferenceSettings_)
{
    processor.enableMentions();
}

std::pair<DispatchedMessageId, DispatchedMessageId>
ConferenceMessageDispatcher::sendMessage(bool isAction, const QString& content)
{
    const auto firstMessageId = nextMessageId;
    auto lastMessageId = firstMessageId;

    for (const auto& message : processor.processOutgoingMessage(isAction, content)) {
        auto messageId = nextMessageId++;
        lastMessageId = messageId;
        if (conference.getPeersCount() != 1) {
            if (message.isAction) {
                messageSender.sendConferenceAction(conference.getId(), message.content);
            } else {
                messageSender.sendConferenceMessage(conference.getId(), message.content);
            }
        }

        // Emit both signals since we do not have receipts for conferences
        //
        // NOTE: We could in theory keep track of our sent message and wait for
        // toxcore to send it back to us to indicate a completed message, but
        // this isn't necessarily the design of toxcore and associating the
        // received message back would be difficult.
        emit messageSent(messageId, message);
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
void ConferenceMessageDispatcher::onMessageReceived(const ToxPk& sender, bool isAction,
                                                    const QString& content)
{
    const bool isSelf = sender == idHandler.getSelfPublicKey();

    if (isSelf) {
        return;
    }

    if (conferenceSettings.getBlockList().contains(sender.toString())) {
        qDebug() << "onConferenceMessageReceived: Filtered:" << sender.toString();
        return;
    }

    emit messageReceived(sender, processor.processIncomingCoreMessage(isAction, content));
}
