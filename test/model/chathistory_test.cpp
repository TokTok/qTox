/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/model/chathistory.h"

#include "src/conferencelist.h"
#include "src/core/icoreidhandler.h"
#include "src/friendlist.h"
#include "src/grouplist.h"
#include "src/model/friend.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/persistence/db/upgrades/dbupgrader.h"
#include "src/persistence/history.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/imessageboxmanager.h"

#include <QObject>
#include <QTemporaryFile>
#include <QtTest/QtTest>

class MockMessageDispatcher : public IMessageDispatcher
{
    Q_OBJECT
public:
    ~MockMessageDispatcher() override;
    std::pair<DispatchedMessageId, DispatchedMessageId> sendMessage(bool isAction,
                                                                    const QString& content) override;
};

MockMessageDispatcher::~MockMessageDispatcher() = default;
std::pair<DispatchedMessageId, DispatchedMessageId>
MockMessageDispatcher::sendMessage(bool isAction, const QString& content)
{
    Q_UNUSED(isAction);
    Q_UNUSED(content);
    return {DispatchedMessageId(0), DispatchedMessageId(0)};
}

class MockMessageBoxManager : public IMessageBoxManager
{
public:
    ~MockMessageBoxManager() override;
    void showInfo(const QString& title, const QString& msg) override
    {
        Q_UNUSED(title);
        Q_UNUSED(msg);
    }
    void showWarning(const QString& title, const QString& msg) override
    {
        Q_UNUSED(title);
        Q_UNUSED(msg);
    }
    void showError(const QString& title, const QString& msg) override
    {
        Q_UNUSED(title);
        Q_UNUSED(msg);
    }
    bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                     bool warning = true, bool yesno = true) override
    {
        Q_UNUSED(title);
        Q_UNUSED(msg);
        Q_UNUSED(warning);
        Q_UNUSED(yesno);
        return defaultAns;
    }
    bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                     const QString& button2, bool defaultAns = false, bool warning = true) override
    {
        Q_UNUSED(title);
        Q_UNUSED(msg);
        Q_UNUSED(button1);
        Q_UNUSED(button2);
        Q_UNUSED(warning);
        return defaultAns;
    }
    void confirmExecutableOpen(const QFileInfo& file) override
    {
        Q_UNUSED(file);
    }
};

MockMessageBoxManager::~MockMessageBoxManager() = default;

class MockCoreIdHandler : public ICoreIdHandler
{
public:
    ~MockCoreIdHandler() override;
    ToxId getSelfId() const override
    {
        return {};
    }
    ToxPk getSelfPublicKey() const override
    {
        return {};
    }
    QString getUsername() const override
    {
        return "Self";
    }
};

MockCoreIdHandler::~MockCoreIdHandler() = default;

class TestChatHistory : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void testHistoryLoading();
    void testHistorySearch();

private:
    std::unique_ptr<QTemporaryFile> dbFile;
    std::shared_ptr<RawDatabase> db;
    std::unique_ptr<MockMessageBoxManager> messageBoxManager;
    std::unique_ptr<Settings> settings;
    std::shared_ptr<History> history;
    std::unique_ptr<MockCoreIdHandler> idHandler;
    std::unique_ptr<MockMessageDispatcher> messageDispatcher;
    std::unique_ptr<FriendList> friendList;
    std::unique_ptr<ConferenceList> conferenceList;
    std::unique_ptr<GroupList> groupList;
    std::unique_ptr<Friend> f;
};

void TestChatHistory::init()
{
    dbFile = std::make_unique<QTemporaryFile>();
    QVERIFY(dbFile->open());
    dbFile->close();
    db = RawDatabase::open(dbFile->fileName(), "", {});
    QVERIFY(DbUpgrader::createCurrentSchema(*db));

    messageBoxManager = std::make_unique<MockMessageBoxManager>();
    settings = std::make_unique<Settings>(*messageBoxManager);
    settings->setEnableLogging(true);
    history = std::make_shared<History>(db, *settings, *messageBoxManager);
    idHandler = std::make_unique<MockCoreIdHandler>();
    messageDispatcher = std::make_unique<MockMessageDispatcher>();
    friendList = std::make_unique<FriendList>();
    conferenceList = std::make_unique<ConferenceList>();
    groupList = std::make_unique<GroupList>();
    f = std::make_unique<Friend>(
        0, ToxPk(QString("FE34BC6D87B66E958C57BBF205F9B79B62BE0AB8A4EFC1F1BB9EC4D0D8FB0663")));
}

void TestChatHistory::cleanup()
{
    f.reset();
    groupList.reset();
    conferenceList.reset();
    friendList.reset();
    messageDispatcher.reset();
    idHandler.reset();
    history.reset();
    settings.reset();
    messageBoxManager.reset();
    db.reset();
    dbFile.reset();
}

void TestChatHistory::testHistoryLoading()
{
    // Add some messages to history
    const ToxPk friendPk(
        QString("FE34BC6D87B66E958C57BBF205F9B79B62BE0AB8A4EFC1F1BB9EC4D0D8FB0663"));
    history->addNewMessage(f->getPersistentId(), "msg1", friendPk, QDateTime::currentDateTime(),
                           true, "Friend");
    history->addNewMessage(f->getPersistentId(), "msg2", friendPk, QDateTime::currentDateTime(),
                           true, "Friend");

    db->sync();

    const ChatHistory chatHistory(*f, history.get(), *idHandler, *settings, *messageDispatcher,
                                  *friendList, *conferenceList, *groupList);

    QCOMPARE(chatHistory.getNextIdx(), ChatLogIdx(2));
    QCOMPARE(chatHistory.at(ChatLogIdx(0)).getContentAsMessage().message.content, QString("msg1"));
    QCOMPARE(chatHistory.at(ChatLogIdx(1)).getContentAsMessage().message.content, QString("msg2"));
}

void TestChatHistory::testHistorySearch()
{
    const ToxPk friendPk(
        QString("FE34BC6D87B66E958C57BBF205F9B79B62BE0AB8A4EFC1F1BB9EC4D0D8FB0663"));
    history->addNewMessage(f->getPersistentId(), "needle", friendPk,
                           QDateTime::currentDateTime().addDays(-1), true, "Friend");
    history->addNewMessage(f->getPersistentId(), "haystack", friendPk, QDateTime::currentDateTime(),
                           true, "Friend");
    db->sync();

    const ChatHistory chatHistory(*f, history.get(), *idHandler, *settings, *messageDispatcher,
                                  *friendList, *conferenceList, *groupList);

    const SearchPos startPos{chatHistory.getNextIdx(), 0};
    const SearchResult result = chatHistory.searchBackward(startPos, "needle", ParameterSearch());

    QVERIFY(result.found);
    QCOMPARE(chatHistory.at(result.pos.logIdx).getContentAsMessage().message.content,
             QString("needle"));
}

QTEST_MAIN(TestChatHistory)
#include "chathistory_test.moc"
