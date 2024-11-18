/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
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
ConferenceMessageDispatcher::sendMessage(bool isAction, QString const& content)
{
    const auto firstMessageId = nextMessageId;
    auto lastMessageId = firstMessageId;

    for (auto const& message : processor.processOutgoingMessage(isAction, content, ExtensionSet())) {
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

std::pair<DispatchedMessageId, DispatchedMessageId>
ConferenceMessageDispatcher::sendExtendedMessage(const QString& content, ExtensionSet extensions)
{
    std::ignore = extensions;
    // Stub this api to immediately fail
    auto messageId = nextMessageId++;
    auto messages = processor.processOutgoingMessage(false, content, ExtensionSet());
    emit messageSent(messageId, messages[0]);
    emit messageBroken(messageId, BrokenMessageReason::unsupportedExtensions);
    return {messageId, messageId};
}

/**
 * @brief Processes and dispatches received message from toxcore
 * @param[in] sender
 * @param[in] isAction True if is action
 * @param[in] content Message content
 */
void ConferenceMessageDispatcher::onMessageReceived(const ToxPk& sender, bool isAction, QString const& content)
{
    bool isSelf = sender == idHandler.getSelfPublicKey();

    if (isSelf) {
        return;
    }

    if (conferenceSettings.getBlackList().contains(sender.toString())) {
        qDebug() << "onConferenceMessageReceived: Filtered:" << sender.toString();
        return;
    }

    emit messageReceived(sender, processor.processIncomingCoreMessage(isAction, content));
}
