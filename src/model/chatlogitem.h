/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/toxfile.h"
#include "src/core/toxpk.h"
#include "src/model/message.h"
#include "src/model/systemmessage.h"
#include "src/persistence/history.h"

#include <memory>

struct ChatLogMessage
{
    MessageState state;
    Message message;
};

struct ChatLogFile
{
    QDateTime timestamp;
    ToxFile file;
};

class ChatLogItem
{
private:
    using ContentPtr = std::unique_ptr<void, void (*)(void*)>;

public:
    enum class ContentType
    {
        message,
        fileTransfer,
        systemMessage,
    };

    ChatLogItem(ToxPk sender_, const QString& displayName_, ChatLogFile file_);
    ChatLogItem(ToxPk sender_, const QString& displayName_, ChatLogMessage message_);
    ChatLogItem(SystemMessage message);
    const ToxPk& getSender() const;
    ContentType getContentType() const;
    ChatLogFile& getContentAsFile();
    const ChatLogFile& getContentAsFile() const;
    ChatLogMessage& getContentAsMessage();
    const ChatLogMessage& getContentAsMessage() const;
    SystemMessage& getContentAsSystemMessage();
    const SystemMessage& getContentAsSystemMessage() const;
    QDateTime getTimestamp() const;
    void setDisplayName(QString name);
    const QString& getDisplayName() const;

private:
    ChatLogItem(ToxPk sender, QString displayName_, ContentType contentType, ContentPtr content);

    ToxPk sender;
    QString displayName;
    ContentType contentType;

    ContentPtr content;
};
