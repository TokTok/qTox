/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "src/model/sessionchatlog.h"

#include "src/core/icoreidhandler.h"
#include "src/model/ichatlog.h"
#include "src/model/imessagedispatcher.h"

#include "src/grouplist.h"

#include <QtTest/QtTest>

#include <memory>

namespace {
const QString TEST_USERNAME = "qTox Tester #1";

Message createMessage(const QString& content)
{
    Message message;
    message.content = content;
    message.isAction = false;
    message.timestamp = QDateTime::currentDateTime();
    return message;
}

class MockCoreIdHandler : public ICoreIdHandler
{
public:
    ToxId getSelfId() const override
    {
        std::terminate();
    }

    ToxPk getSelfPublicKey() const override
    {
        static uint8_t id[TOX_PUBLIC_KEY_SIZE] = {5};
        return ToxPk(id);
    }

    QString getUsername() const override
    {
        return TEST_USERNAME;
    }
};
} // namespace

class TestSessionChatLog : public QObject
{
    Q_OBJECT

public:
    TestSessionChatLog() = default;

private slots:
    void init();

    void testSanity();
    void testMessageStatus();
    void testFileTransferStatus();

private:
    MockCoreIdHandler idHandler;
    std::unique_ptr<SessionChatLog> chatLog;
    std::unique_ptr<FriendList> friendList;
    std::unique_ptr<ConferenceList> conferenceList;
    std::unique_ptr<GroupList> groupList;
};

/**
 * @brief Test initialization, resets the chat log
 */
void TestSessionChatLog::init()
{
    friendList = std::make_unique<FriendList>();
    conferenceList = std::make_unique<ConferenceList>();
    groupList = std::make_unique<GroupList>();
    chatLog = std::make_unique<SessionChatLog>(idHandler, *friendList, *conferenceList, *groupList);
}

/**
 * @brief Tests basic message status transitions (pending -> complete/broken)
 */
void TestSessionChatLog::testMessageStatus()
{
    const DispatchedMessageId id(42);
    chatLog->onMessageSent(id, createMessage("status test"));

    const ChatLogIdx idx(0);
    QCOMPARE(chatLog->getNextIdx(), ChatLogIdx(1));
    const auto& item = chatLog->at(idx);
    QVERIFY(item.getContentType() == ChatLogItem::ContentType::message);
    QCOMPARE(item.getContentAsMessage().state, MessageState::pending);

    // Complete the message
    chatLog->onMessageComplete(id);
    QCOMPARE(chatLog->at(idx).getContentAsMessage().state, MessageState::complete);

    // Test broken status
    const DispatchedMessageId id2(43);
    chatLog->onMessageSent(id2, createMessage("broken test"));
    const ChatLogIdx idx2(1);
    QCOMPARE(chatLog->at(idx2).getContentAsMessage().state, MessageState::pending);

    chatLog->onMessageBroken(id2, BrokenMessageReason::unknown);
    QCOMPARE(chatLog->at(idx2).getContentAsMessage().state, MessageState::broken);
}

/**
 * @brief Tests file transfer status updates
 */
void TestSessionChatLog::testFileTransferStatus()
{
    ToxFile file;
    file.fileNum = 123;
    file.fileName = "test.txt";
    file.status = ToxFile::INITIALIZING;

    chatLog->onFileUpdated(ToxPk(), file);
    const ChatLogIdx idx(0);
    QVERIFY(chatLog->at(idx).getContentType() == ChatLogItem::ContentType::fileTransfer);
    QCOMPARE(chatLog->at(idx).getContentAsFile().file.status, ToxFile::INITIALIZING);

    // Update status
    file.status = ToxFile::TRANSMITTING;
    chatLog->onFileUpdated(ToxPk(), file);
    QCOMPARE(chatLog->at(idx).getContentAsFile().file.status, ToxFile::TRANSMITTING);

    file.status = ToxFile::PAUSED;
    chatLog->onFileUpdated(ToxPk(), file);
    QCOMPARE(chatLog->at(idx).getContentAsFile().file.status, ToxFile::PAUSED);
}

/**
 * @brief Quick sanity test that the chat log is working as expected. Tests basic insertion,
 * retrieval, and searching of messages
 */
void TestSessionChatLog::testSanity()
{
    /* ChatLogIdx(0) */ chatLog->onMessageSent(DispatchedMessageId(0), createMessage("test"));
    /* ChatLogIdx(1) */ chatLog->onMessageSent(DispatchedMessageId(1), createMessage("test test"));
    /* ChatLogIdx(2) */ chatLog->onMessageReceived(ToxPk(), createMessage("test2"));
    /* ChatLogIdx(3) */ chatLog->onFileUpdated(ToxPk(), ToxFile());
    /* ChatLogIdx(4) */ chatLog->onMessageSent(DispatchedMessageId(2), createMessage("test3"));
    /* ChatLogIdx(5) */ chatLog->onMessageSent(DispatchedMessageId(3), createMessage("test4"));
    /* ChatLogIdx(6) */ chatLog->onMessageSent(DispatchedMessageId(4), createMessage("test"));
    /* ChatLogIdx(7) */ chatLog->onMessageReceived(ToxPk(), createMessage("test5"));

    QVERIFY(chatLog->getNextIdx() == ChatLogIdx(8));
    QVERIFY(chatLog->at(ChatLogIdx(3)).getContentType() == ChatLogItem::ContentType::fileTransfer);
    QVERIFY(chatLog->at(ChatLogIdx(7)).getContentType() == ChatLogItem::ContentType::message);

    auto searchPos = SearchPos{ChatLogIdx(1), 0};
    auto searchResult = chatLog->searchForward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(1));
    QVERIFY(searchResult.start == 0);

    searchPos = searchResult.pos;
    searchResult = chatLog->searchForward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(1));
    QVERIFY(searchResult.start == 5);

    searchPos = searchResult.pos;
    searchResult = chatLog->searchForward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(2));
    QVERIFY(searchResult.start == 0);

    searchPos = searchResult.pos;
    searchResult = chatLog->searchBackward(searchPos, "test", ParameterSearch());

    QVERIFY(searchResult.found);
    QVERIFY(searchResult.len == 4);
    QVERIFY(searchResult.pos.logIdx == ChatLogIdx(1));
    QVERIFY(searchResult.start == 5);
}

QTEST_GUILESS_MAIN(TestSessionChatLog)
#include "sessionchatlog_test.moc"
