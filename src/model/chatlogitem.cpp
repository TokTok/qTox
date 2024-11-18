/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "chatlogitem.h"
#include "src/model/friend.h"
#include "src/model/group.h"

#include <cassert>
#include <utility>

namespace {

/**
 * Helper template to get the correct deleter function for our type erased unique_ptr
 */
template <typename T>
struct ChatLogItemDeleter
{
    static void doDelete(void* ptr)
    {
        delete static_cast<T*>(ptr);
    }
};
} // namespace

ChatLogItem::ChatLogItem(ToxPk sender_, const QString& displayName_, ChatLogFile file_)
    : ChatLogItem(std::move(sender_), displayName_, ContentType::fileTransfer,
                  ContentPtr(new ChatLogFile(std::move(file_)),
                             ChatLogItemDeleter<ChatLogFile>::doDelete))
{
}

ChatLogItem::ChatLogItem(ToxPk sender_, const QString& displayName_, ChatLogMessage message_)
    : ChatLogItem(sender_, displayName_, ContentType::message,
                  ContentPtr(new ChatLogMessage(std::move(message_)),
                             ChatLogItemDeleter<ChatLogMessage>::doDelete))
{
}

ChatLogItem::ChatLogItem(SystemMessage systemMessage)
    : contentType(ContentType::systemMessage)
    , content(new SystemMessage(std::move(systemMessage)), ChatLogItemDeleter<SystemMessage>::doDelete)
{
}

ChatLogItem::ChatLogItem(ToxPk sender_, QString displayName_, ContentType contentType_,
                         ContentPtr content_)
    : sender(std::move(sender_))
    , displayName(std::move(displayName_))
    , contentType(contentType_)
    , content(std::move(content_))
{
}

const ToxPk& ChatLogItem::getSender() const
{
    return sender;
}

ChatLogItem::ContentType ChatLogItem::getContentType() const
{
    return contentType;
}

ChatLogFile& ChatLogItem::getContentAsFile()
{
    assert(contentType == ContentType::fileTransfer);
    return *static_cast<ChatLogFile*>(content.get());
}

const ChatLogFile& ChatLogItem::getContentAsFile() const
{
    assert(contentType == ContentType::fileTransfer);
    return *static_cast<ChatLogFile*>(content.get());
}

ChatLogMessage& ChatLogItem::getContentAsMessage()
{
    assert(contentType == ContentType::message);
    return *static_cast<ChatLogMessage*>(content.get());
}

const ChatLogMessage& ChatLogItem::getContentAsMessage() const
{
    assert(contentType == ContentType::message);
    return *static_cast<ChatLogMessage*>(content.get());
}

SystemMessage& ChatLogItem::getContentAsSystemMessage()
{
    assert(contentType == ContentType::systemMessage);
    return *static_cast<SystemMessage*>(content.get());
}

const SystemMessage& ChatLogItem::getContentAsSystemMessage() const
{
    assert(contentType == ContentType::systemMessage);
    return *static_cast<SystemMessage*>(content.get());
}


QDateTime ChatLogItem::getTimestamp() const
{
    switch (contentType) {
    case ChatLogItem::ContentType::message: {
        const auto& message = getContentAsMessage();
        return message.message.timestamp;
    }
    case ChatLogItem::ContentType::fileTransfer: {
        const auto& file = getContentAsFile();
        return file.timestamp;
    }
    case ChatLogItem::ContentType::systemMessage: {
        const auto& systemMessage = getContentAsSystemMessage();
        return systemMessage.timestamp;
    }
    }

    assert(false);
    return QDateTime();
}

void ChatLogItem::setDisplayName(QString name)
{
    displayName = name;
}

const QString& ChatLogItem::getDisplayName() const
{
    return displayName;
}
