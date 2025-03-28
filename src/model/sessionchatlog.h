/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "ichatlog.h"
#include "imessagedispatcher.h"

#include <QList>
#include <QObject>

struct SessionChatLogMetadata;
class ICoreIdHandler;
class FriendList;
class ConferenceList;

class SessionChatLog : public IChatLog
{
    Q_OBJECT
public:
    SessionChatLog(const ICoreIdHandler& coreIdHandler_, FriendList& friendList,
                   ConferenceList& conferenceList);
    SessionChatLog(ChatLogIdx initialIdx, const ICoreIdHandler& coreIdHandler_,
                   FriendList& friendList, ConferenceList& conferenceList);

    ~SessionChatLog() override;
    const ChatLogItem& at(ChatLogIdx idx) const override;
    SearchResult searchForward(SearchPos startPos, const QString& phrase,
                               const ParameterSearch& parameter) const override;
    SearchResult searchBackward(SearchPos startPos, const QString& phrase,
                                const ParameterSearch& parameter) const override;
    ChatLogIdx getFirstIdx() const override;
    ChatLogIdx getNextIdx() const override;
    std::vector<DateChatLogIdxPair> getDateIdxs(const QDate& startDate, size_t maxDates) const override;
    void addSystemMessage(const SystemMessage& message) override;

    void insertCompleteMessageAtIdx(ChatLogIdx idx, const ToxPk& sender, QString senderName,
                                    const ChatLogMessage& message);
    void insertIncompleteMessageAtIdx(ChatLogIdx idx, const ToxPk& sender, QString senderName,
                                      const ChatLogMessage& message, DispatchedMessageId dispatchId);
    void insertBrokenMessageAtIdx(ChatLogIdx idx, const ToxPk& sender, QString senderName,
                                  const ChatLogMessage& message);
    void insertFileAtIdx(ChatLogIdx idx, const ToxPk& sender, QString senderName,
                         const ChatLogFile& file);
    void insertSystemMessageAtIdx(ChatLogIdx idx, SystemMessage message);

public slots:
    void onMessageReceived(const ToxPk& sender, const Message& message);
    void onMessageSent(DispatchedMessageId id, const Message& message);
    void onMessageComplete(DispatchedMessageId id);
    void onMessageBroken(DispatchedMessageId id, BrokenMessageReason reason);

    void onFileUpdated(const ToxPk& sender, const ToxFile& file);
    void onFileTransferRemotePausedUnpaused(const ToxPk& sender, const ToxFile& file, bool paused);
    void onFileTransferBrokenUnbroken(const ToxPk& sender, const ToxFile& file, bool broken);


private:
    QString resolveSenderNameFromSender(const ToxPk& sender);


private:
    const ICoreIdHandler& coreIdHandler;

    ChatLogIdx nextIdx = ChatLogIdx(0);

    std::map<ChatLogIdx, ChatLogItem> items;

    struct CurrentFileTransfer
    {
        ChatLogIdx idx;
        ToxFile file;
    };

    /**
     * Short list of active file transfers in given log. This is to make it
     * so we don't have to search through all files that have ever been transferred
     * in order to find our existing transfers
     */
    std::vector<CurrentFileTransfer> currentFileTransfers;

    /**
     * Maps DispatchedMessageIds back to ChatLogIdxs. Messages are removed when the message
     * is marked as completed
     */
    QMap<DispatchedMessageId, ChatLogIdx> outgoingMessages;
    FriendList& friendList;
    ConferenceList& conferenceList;
};
