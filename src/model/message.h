/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QDateTime>
#include <QRegularExpression>
#include <QString>

#include <vector>

class Friend;

// NOTE: This could be extended in the future to handle all text processing (see
// ChatMessage::createChatMessage)
enum class MessageMetadataType
{
    selfMention,
};

// May need to be extended in the future to have a more variant-like type (imagine
// if we wanted to add message replies and shoved a reply id in here)
struct MessageMetadata
{
    MessageMetadataType type;
    // Indicates start position within a Message::content
    size_t start{0};
    // Indicates end position within a Message::content
    size_t end{0};
};

struct Message
{
    bool isAction{false};
    QString content;
    QDateTime timestamp;
    std::vector<MessageMetadata> metadata;
};


class MessageProcessor
{

public:
    /**
     * Parameters needed by all message processors. Used to reduce duplication
     * of expensive data looked at by all message processors
     */
    class SharedParams
    {

    public:
        explicit SharedParams(uint64_t maxCoreMessageSize_)
            : maxCoreMessageSize(maxCoreMessageSize_)
        {
        }

        QRegularExpression getNameMention() const
        {
            return nameMention;
        }
        QRegularExpression getSanitizedNameMention() const
        {
            return sanitizedNameMention;
        }
        QRegularExpression getPublicKeyMention() const
        {
            return pubKeyMention;
        }
        void onUserNameSet(const QString& username);
        void setPublicKey(const QString& pk);

        uint64_t getMaxCoreMessageSize() const
        {
            return maxCoreMessageSize;
        }

    private:
        uint64_t maxCoreMessageSize;
        QRegularExpression nameMention;
        QRegularExpression sanitizedNameMention;
        QRegularExpression pubKeyMention;
    };

    MessageProcessor(const SharedParams& sharedParams_);

    std::vector<Message> processOutgoingMessage(bool isAction, const QString& content);
    Message processIncomingCoreMessage(bool isAction, const QString& message);

    /**
     * @brief Enables mention detection in the processor
     */
    inline void enableMentions()
    {
        detectingMentions = true;
    }

    /**
     * @brief Disables mention detection in the processor
     */
    inline void disableMentions()
    {
        detectingMentions = false;
    }

private:
    bool detectingMentions = false;
    const SharedParams& sharedParams;
};
