/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "chatline.h"

#include "src/core/toxfile.h"
#include "src/persistence/history.h"

#include <QDateTime>

class CoreFile;
class DocumentCache;
class IMessageBoxManager;
class QGraphicsScene;
class ImageLoader;
class Settings;
class SmileyPack;
class Style;

class ChatMessage : public ChatLine
{
public:
    using Ptr = std::shared_ptr<ChatMessage>;

    enum SystemMessageType
    {
        INFO,
        ERROR,
        TYPING,
    };

    enum MessageType
    {
        NORMAL,
        ACTION,
        ALERT,
    };

    ChatMessage(DocumentCache& documentCache, Settings& settings, Style& style);
    ~ChatMessage() override;
    ChatMessage(const ChatMessage&) = default;
    ChatMessage(ChatMessage&&) = default;

    static ChatMessage::Ptr createChatMessage(const QString& sender, const QString& rawMessage,
                                              MessageType type, bool isMe, MessageState state,
                                              const QDateTime& date, DocumentCache& documentCache,
                                              SmileyPack& smileyPack, Settings& settings,
                                              Style& style, bool colorizeName = false);
    static ChatMessage::Ptr createChatInfoMessage(const QString& rawMessage, SystemMessageType type,
                                                  const QDateTime& date, DocumentCache& documentCache,
                                                  Settings& settings, Style& style);
    static ChatMessage::Ptr createFileTransferMessage(const QString& sender, CoreFile& coreFile,
                                                      ToxFile file, bool isMe, const QDateTime& date,
                                                      DocumentCache& documentCache,
                                                      Settings& settings, Style& style,
                                                      IMessageBoxManager& messageBoxManager, ImageLoader& imageLoader);
    static ChatMessage::Ptr createTypingNotification(DocumentCache& documentCache,
                                                     Settings& settings, Style& style);
    static ChatMessage::Ptr createBusyNotification(DocumentCache& documentCache, Settings& settings,
                                                   Style& style);

    void markAsDelivered(const QDateTime& time);
    void markAsBroken();
    QString toString() const;
    bool isAction() const;
    void setAsAction();
    void hideSender();
    void hideDate();

protected:
    static QString detectQuotes(const QString& str, MessageType type);
    static QString wrapDiv(const QString& str, const QString& div);

private:
    bool action = false;
    DocumentCache& documentCache;
    Settings& settings;
    Style& style;
};
