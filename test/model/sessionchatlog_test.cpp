/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/model/ichatlog.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/sessionchatlog.h"

#include <QtTest/QtTest>

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
        return ToxId();
    }

    ToxPk getSelfPublicKey() const override
    {
        static uint8_t id[ToxPk::size] = {5};
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
    TestSessionChatLog() {}

private slots:
    void init();

    void testSanity();

private:
    MockCoreIdHandler idHandler;
    std::unique_ptr<SessionChatLog> chatLog;
    std::unique_ptr<FriendList> friendList;
    std::unique_ptr<GroupList> groupList;
};

/**
 * @brief Test initialiation, resets the chatlog
 */
void TestSessionChatLog::init()
{
    friendList = std::unique_ptr<FriendList>(new FriendList());
    groupList = std::unique_ptr<GroupList>(new GroupList());
    chatLog = std::unique_ptr<SessionChatLog>(new SessionChatLog(idHandler, *friendList, *groupList));
}

/**
 * @brief Quick sanity test that the chatlog is working as expected. Tests basic insertion, retrieval, and searching of messages
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
