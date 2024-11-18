/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "ichatlog.h"
#include "sessionchatlog.h"
#include "src/model/brokenmessagereason.h"
#include "src/persistence/history.h"

#include <QSet>

class Settings;
class FriendList;
class ConferenceList;

class ChatHistory : public IChatLog
{
    Q_OBJECT
public:
    ChatHistory(Chat& chat_, History* history_, const ICoreIdHandler& coreIdHandler_,
                const Settings& settings_, IMessageDispatcher& messageDispatcher,
                FriendList& friendList, ConferenceList& conferenceList);
    const ChatLogItem& at(ChatLogIdx idx) const override;
    SearchResult searchForward(SearchPos startIdx, const QString& phrase,
                               const ParameterSearch& parameter) const override;
    SearchResult searchBackward(SearchPos startIdx, const QString& phrase,
                                const ParameterSearch& parameter) const override;
    ChatLogIdx getFirstIdx() const override;
    ChatLogIdx getNextIdx() const override;
    std::vector<DateChatLogIdxPair> getDateIdxs(const QDate& startDate, size_t maxDates) const override;
    void addSystemMessage(const SystemMessage& message) override;

public slots:
    void onFileUpdated(const ToxPk& sender, const ToxFile& file);
    void onFileTransferRemotePausedUnpaused(const ToxPk& sender, const ToxFile& file, bool paused);
    void onFileTransferBrokenUnbroken(const ToxPk& sender, const ToxFile& file, bool broken);


private slots:
    void onMessageReceived(const ToxPk& sender, const Message& message);
    void onMessageSent(DispatchedMessageId id, const Message& message);
    void onMessageComplete(DispatchedMessageId id);
    void onMessageBroken(DispatchedMessageId id, BrokenMessageReason reason);

private:
    void ensureIdxInSessionChatLog(ChatLogIdx idx) const;
    void loadHistoryIntoSessionChatLog(ChatLogIdx start) const;
    void dispatchUnsentMessages(IMessageDispatcher& messageDispatcher);
    void handleDispatchedMessage(DispatchedMessageId dispatchId, RowId historyId);
    void completeMessage(DispatchedMessageId id);
    void breakMessage(DispatchedMessageId id, BrokenMessageReason reason);
    bool canUseHistory() const;
    ChatLogIdx getInitialChatLogIdx() const;

    Chat& chat;
    History* history;
    const Settings& settings;
    const ICoreIdHandler& coreIdHandler;
    mutable SessionChatLog sessionChatLog;

    // If a message completes before it's inserted into history it will end up
    // in this set
    QSet<DispatchedMessageId> completedMessages;
    // If a message breaks before it's inserted into history it will end up
    // in this set
    QMap<DispatchedMessageId, BrokenMessageReason> brokenMessages;

    // If a message is inserted into history before it gets a completion
    // callback it will end up in this map
    QMap<DispatchedMessageId, RowId> dispatchedMessageRowIdMap;
};
