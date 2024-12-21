/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "message.h"
#include "friend.h"
#include "src/core/core.h"

#include <cassert>

namespace {
QStringList splitMessage(const QString& message, uint64_t maxLength)
{
    QStringList splittedMsgs;
    QByteArray ba_message{message.toUtf8()};
    while (static_cast<uint64_t>(ba_message.size()) > maxLength) {
        int splitPos = ba_message.lastIndexOf('\n', maxLength - 1);

        if (splitPos <= 0) {
            splitPos = ba_message.lastIndexOf(' ', maxLength - 1);
        }

        if (splitPos <= 0) {
            constexpr uint8_t firstOfMultiByteMask = 0xC0;
            constexpr uint8_t multiByteMask = 0x80;
            splitPos = maxLength;
            // don't split a utf8 character
            if ((ba_message[splitPos] & multiByteMask) == multiByteMask) {
                while ((ba_message[splitPos] & firstOfMultiByteMask) != firstOfMultiByteMask) {
                    --splitPos;
                }
            }
            --splitPos;
        }
        splittedMsgs.append(QString::fromUtf8(ba_message.left(splitPos + 1)));
        ba_message = ba_message.mid(splitPos + 1);
    }

    splittedMsgs.append(QString::fromUtf8(ba_message));
    return splittedMsgs;
}
} // namespace
void MessageProcessor::SharedParams::onUserNameSet(const QString& username)
{
    QString sanitizedName = username;
    sanitizedName.remove(QRegularExpression("[\\t\\n\\v\\f\\r\\x0000]"));
    nameMention = QRegularExpression("\\b" + QRegularExpression::escape(username) + "\\b",
                                     QRegularExpression::CaseInsensitiveOption);
    sanitizedNameMention = QRegularExpression("\\b" + QRegularExpression::escape(sanitizedName) + "\\b",
                                              QRegularExpression::CaseInsensitiveOption);
}

/**
 * @brief Set the public key on which a message should be highlighted
 * @param pk ToxPk in its hex string form
 */
void MessageProcessor::SharedParams::setPublicKey(const QString& pk)
{
    // no sanitization needed, we expect a ToxPk in its string form
    pubKeyMention = QRegularExpression("\\b" + pk + "\\b", QRegularExpression::CaseInsensitiveOption);
}

MessageProcessor::MessageProcessor(const MessageProcessor::SharedParams& sharedParams_)
    : sharedParams(sharedParams_)
{
}

/**
 * @brief Converts an outgoing message into one (or many) sanitized Message(s)
 */
std::vector<Message> MessageProcessor::processOutgoingMessage(bool isAction, const QString& content)
{
    std::vector<Message> ret;

    const auto maxSendingSize = sharedParams.getMaxCoreMessageSize();

    const auto splitMsgs = splitMessage(content, maxSendingSize);

    ret.reserve(splitMsgs.size());

    QDateTime timestamp = QDateTime::currentDateTime();
    std::transform(splitMsgs.begin(), splitMsgs.end(), std::back_inserter(ret),
                   [&](const QString& part) {
                       Message message;
                       message.isAction = isAction;
                       message.content = part;
                       message.timestamp = timestamp;
                       return message;
                   });

    return ret;
}

/**
 * @brief Converts an incoming message into a sanitized Message
 */
Message MessageProcessor::processIncomingCoreMessage(bool isAction, const QString& message)
{
    QDateTime timestamp = QDateTime::currentDateTime();
    auto ret = Message{};
    ret.isAction = isAction;
    ret.content = message;
    ret.timestamp = timestamp;

    if (detectingMentions) {
        auto nameMention = sharedParams.getNameMention();
        auto sanitizedNameMention = sharedParams.getSanitizedNameMention();
        auto pubKeyMention = sharedParams.getPublicKeyMention();

        for (const auto& mention : {nameMention, sanitizedNameMention, pubKeyMention}) {
            auto matchIt = mention.globalMatch(ret.content);
            if (!matchIt.hasNext()) {
                continue;
            }

            auto match = matchIt.next();

            auto pos = static_cast<size_t>(match.capturedStart());
            auto length = static_cast<size_t>(match.capturedLength());

            // skip matches on empty usernames
            if (length == 0) {
                continue;
            }

            ret.metadata.push_back({MessageMetadataType::selfMention, pos, pos + length});
            break;
        }
    }

    return ret;
}
