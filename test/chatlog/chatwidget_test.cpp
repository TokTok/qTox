/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/chatlog/chatwidget.h"

#include "src/chatlog/chatlinestorage.h"
#include "src/chatlog/documentcache.h"
#include "src/chatlog/pixmapcache.h"
#include "src/core/icoreidhandler.h"
#include "src/model/ichatlog.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/style.h"
#include "src/widget/tool/imessageboxmanager.h"

#include <QApplication>
#include <QObject>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class MockChatLog : public IChatLog
{
    Q_OBJECT
public:
    explicit MockChatLog(size_t numMessages);
    ~MockChatLog() override;

    const ChatLogItem& at(ChatLogIdx idx) const override;
    SearchResult searchForward(SearchPos pos, const QString& phrase,
                               const ParameterSearch& parameter) const override;
    SearchResult searchBackward(SearchPos pos, const QString& phrase,
                                const ParameterSearch& parameter) const override;
    ChatLogIdx getFirstIdx() const override;
    ChatLogIdx getNextIdx() const override;
    std::vector<IChatLog::DateChatLogIdxPair> getDateIdxs(const QDate& startDate,
                                                          size_t maxDates) const override;
    void addSystemMessage(const SystemMessage& message) override;

    void addMessage(const QString& content);
    void addFileTransfer();

private:
    std::vector<ChatLogItem> items;
};

MockChatLog::MockChatLog(size_t numMessages)
{
    for (size_t i = 0; i < numMessages; ++i) {
        Message msg;
        msg.content = QString("Message %1").arg(i);
        // Every 10 messages, increment the day
        msg.timestamp = QDateTime(QDate(2020, 1, 1).addDays(i / 10), QTime(12, 0)).addSecs(i % 10);
        items.emplace_back(ToxPk(), QString("Sender %1").arg(i % 5),
                           ChatLogMessage{MessageState::complete, msg});
    }
}

MockChatLog::~MockChatLog() = default;

const ChatLogItem& MockChatLog::at(ChatLogIdx idx) const
{
    return items.at(idx.get());
}

SearchResult MockChatLog::searchForward(SearchPos pos, const QString& phrase,
                                        const ParameterSearch& parameter) const
{
    std::ignore = parameter;
    for (size_t i = pos.logIdx.get(); i < items.size(); ++i) {
        const auto& item = items[i];
        if (item.getContentType() == ChatLogItem::ContentType::message) {
            const auto& content = item.getContentAsMessage().message.content;
            if (content.contains(phrase)) {
                SearchResult result;
                result.found = true;
                result.pos.logIdx = ChatLogIdx(i);
                result.start = content.indexOf(phrase);
                result.len = phrase.length();
                return result;
            }
        }
    }
    return {};
}

SearchResult MockChatLog::searchBackward(SearchPos pos, const QString& phrase,
                                         const ParameterSearch& parameter) const
{
    std::ignore = parameter;
    size_t start = pos.logIdx.get();
    if (start >= items.size())
        start = items.size() - 1;
    for (int i = static_cast<int>(start); i >= 0; --i) {
        const auto& item = items[i];
        if (item.getContentType() == ChatLogItem::ContentType::message) {
            const auto& content = item.getContentAsMessage().message.content;
            if (content.contains(phrase)) {
                SearchResult result;
                result.found = true;
                result.pos.logIdx = ChatLogIdx(i);
                result.start = content.indexOf(phrase);
                result.len = phrase.length();
                return result;
            }
        }
    }
    return {};
}

ChatLogIdx MockChatLog::getFirstIdx() const
{
    return ChatLogIdx(0);
}

ChatLogIdx MockChatLog::getNextIdx() const
{
    return ChatLogIdx(items.size());
}

std::vector<IChatLog::DateChatLogIdxPair> MockChatLog::getDateIdxs(const QDate& startDate,
                                                                   size_t maxDates) const
{
    std::vector<IChatLog::DateChatLogIdxPair> result;
    for (size_t i = 0; i < items.size(); i += 10) {
        auto date = items[i].getTimestamp().date();
        if (date >= startDate) {
            result.push_back({date, ChatLogIdx(i)});
            if (result.size() >= maxDates)
                break;
        }
    }
    return result;
}

void MockChatLog::addSystemMessage(const SystemMessage& message)
{
    const ChatLogIdx newIdx(items.size());
    items.emplace_back(message);
    emit itemUpdated(newIdx);
}

void MockChatLog::addMessage(const QString& content)
{
    Message msg;
    msg.content = content;
    msg.timestamp = QDateTime::currentDateTime();
    const ChatLogIdx newIdx(items.size());
    items.emplace_back(ToxPk(), "Sender", ChatLogMessage{MessageState::complete, msg});
    emit itemUpdated(newIdx);
}

void MockChatLog::addFileTransfer()
{
    ChatLogFile file;
    file.timestamp = QDateTime::currentDateTime();
    file.file.fileName = "test.txt";
    file.file.resumeFileId = QByteArray("\x01\x02\x03\x04", 4);
    file.file.fileKind = 0;
    file.file.status = ToxFile::TRANSMITTING;

    const ChatLogIdx newIdx(items.size());
    items.emplace_back(ToxPk(), "Sender", file);
    emit itemUpdated(newIdx);
}

class MockMessageBoxManager : public IMessageBoxManager
{
public:
    ~MockMessageBoxManager() override;
    void showInfo(const QString& title, const QString& msg) override
    {
        std::ignore = title;
        std::ignore = msg;
    }
    void showWarning(const QString& title, const QString& msg) override
    {
        std::ignore = title;
        std::ignore = msg;
    }
    void showError(const QString& title, const QString& msg) override
    {
        std::ignore = title;
        std::ignore = msg;
    }
    bool askQuestion(const QString& title, const QString& msg, bool d = false, bool warning = true,
                     bool yesno = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = warning;
        std::ignore = yesno;
        return d;
    }
    bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                     const QString& button2, bool d = false, bool warning = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = button1;
        std::ignore = button2;
        std::ignore = warning;
        return d;
    }
    void confirmExecutableOpen(const QFileInfo& file) override
    {
        std::ignore = file;
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

class MockChatWidget : public ChatWidget
{
public:
    using ChatWidget::ChatWidget;
    ~MockChatWidget() override;
    bool mockActiveTransfer = false;
    bool isActiveFileTransfer(ChatLine::Ptr /*l*/) override
    {
        return mockActiveTransfer;
    }
};

MockChatWidget::~MockChatWidget() = default;

class TestChatWidget : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void testInitialState();
    void testScrollUpPagination();
    void testScrollDownPagination();
    void testJumpToIdx();
    void testRapidScrolling();
    void testWindowSizeLimitation();
    void testStickToBottom();
    void testSearch();
    void testSelection();
    void testTypingNotification();
    void testJumpToDate();
    void testClear();
    void testSystemMessage();
    void testScrollStress();
    void testResizeAndInterrupt();
    void testClearPreservesFileTransfers();

private:
    std::unique_ptr<QTemporaryDir> tempHome;
    std::unique_ptr<MockMessageBoxManager> messageBoxManager;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<Style> style;
    std::unique_ptr<SmileyPack> smileyPack;
    std::unique_ptr<DocumentCache> documentCache;
    std::unique_ptr<MockCoreIdHandler> idHandler;
    std::unique_ptr<MockChatLog> chatLog;
    std::unique_ptr<MockChatWidget> chatWidget;

    void waitForRender(QSignalSpy& spy)
    {
        if (spy.isEmpty()) {
            spy.wait(1000);
        }
        for (int i = 0; i < 5; ++i) {
            QApplication::processEvents();
        }
        spy.clear();
    }

    int getRenderedLineCount() const;
};

void TestChatWidget::init()
{
    tempHome = std::make_unique<QTemporaryDir>();
    qputenv("HOME", tempHome->path().toUtf8());
    qputenv("XDG_CONFIG_HOME", tempHome->path().toUtf8());

    messageBoxManager = std::make_unique<MockMessageBoxManager>();
    settings = std::make_unique<Settings>(*messageBoxManager);

    // Use smaller window sizes for faster tests
    settings->setChatMaxWindowSize(50);
    settings->setChatWindowChunkSize(20);

    style = std::make_unique<Style>();
    smileyPack = std::make_unique<SmileyPack>(*settings);
    documentCache = std::make_unique<DocumentCache>(*smileyPack, *settings);
    idHandler = std::make_unique<MockCoreIdHandler>();
    chatLog = std::make_unique<MockChatLog>(500);

    chatWidget = std::make_unique<MockChatWidget>(*chatLog, *idHandler, nullptr, *documentCache,
                                                  *smileyPack, *settings, *style, *messageBoxManager);
    chatWidget->show();
    chatWidget->resize(400, 300);

    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);
    waitForRender(spy);
}

void TestChatWidget::cleanup()
{
    chatWidget.reset();
    chatLog.reset();
    idHandler.reset();
    documentCache.reset();
    smileyPack.reset();
    style.reset();
    settings.reset();
    messageBoxManager.reset();
    tempHome.reset();
}

int TestChatWidget::getRenderedLineCount() const
{
    // scene is shadowed in ChatWidget, so we need to cast to QGraphicsView
    return static_cast<const QGraphicsView*>(chatWidget.get())->scene()->items().size();
}

void TestChatWidget::testInitialState()
{
    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    // Should be at bottom initially
    QVERIFY(vScroll->value() >= vScroll->maximum() - 1);

    // verify we have messages
    QVERIFY(vScroll->maximum() > 0);
}

void TestChatWidget::testScrollUpPagination()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    const ChatLogIdx previousStart = chatWidget->getRenderedStartIdx();

    // Scroll to top
    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    vScroll->setValue(vScroll->minimum());
    waitForRender(spy);

    // Should have loaded earlier messages
    QVERIFY(chatWidget->getRenderedStartIdx() < previousStart);
    // Position can be minimum if we hit the top
}

void TestChatWidget::testScrollDownPagination()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    // Jump to the beginning
    chatWidget->jumpToIdx(ChatLogIdx(0));
    waitForRender(spy);

    const ChatLogIdx previousEnd = chatWidget->getRenderedEndIdx();

    // Scroll to bottom
    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    vScroll->setValue(vScroll->maximum());
    waitForRender(spy);

    // Should have loaded later messages
    QVERIFY(chatWidget->getRenderedEndIdx() > previousEnd);
}

void TestChatWidget::testJumpToIdx()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    // Jump to far from either end (log has 500 messages)
    chatWidget->jumpToIdx(ChatLogIdx(25));
    waitForRender(spy);

    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    // Should be somewhere in the middle
    QVERIFY(vScroll->value() > vScroll->minimum());
    QVERIFY(vScroll->value() < vScroll->maximum());
}

void TestChatWidget::testRapidScrolling()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    QScrollBar* vScroll = chatWidget->verticalScrollBar();

    for (int i = 0; i < 5; ++i) {
        vScroll->setValue(vScroll->minimum());
        QTest::qWait(20);
        vScroll->setValue(vScroll->maximum());
        QTest::qWait(20);
    }

    waitForRender(spy);
    QVERIFY(vScroll->maximum() > 0);
}

void TestChatWidget::testWindowSizeLimitation()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    // Scroll up many times
    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    for (int i = 0; i < 10; ++i) {
        const ChatLogIdx previousStart = chatWidget->getRenderedStartIdx();
        vScroll->setValue(vScroll->minimum());

        // Only wait if we expect a change
        if (previousStart > ChatLogIdx(0)) {
            waitForRender(spy);
        } else {
            QTest::qWait(100);
        }
    }

    // The number of items in the scene should be limited.
    // Each ChatLine (message) has 3 items in the scene.
    // maxWindowSize is 50, so ~150 items for messages, plus date lines.
    QVERIFY(getRenderedLineCount() < 200);
}

void TestChatWidget::testStickToBottom()
{
    // Use a fresh widget with a small log to avoid pagination interference
    chatLog = std::make_unique<MockChatLog>(20);
    chatWidget = std::make_unique<MockChatWidget>(*chatLog, *idHandler, nullptr, *documentCache,
                                                  *smileyPack, *settings, *style, *messageBoxManager);
    chatWidget->show();
    chatWidget->resize(400, 300);

    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);
    waitForRender(spy);

    QScrollBar* vScroll = chatWidget->verticalScrollBar();

    // 1. Scroll away from bottom
    vScroll->setValue(vScroll->minimum());
    waitForRender(spy);
    const int posBefore = vScroll->value();

    // 2. Add message, should NOT scroll to bottom
    chatLog->addMessage("Hello");
    waitForRender(spy);
    QVERIFY(vScroll->value() < vScroll->maximum());
    QCOMPARE(vScroll->value(), posBefore);

    // 3. Scroll to bottom
    vScroll->setValue(vScroll->maximum());
    QApplication::processEvents();
    waitForRender(spy);

    // 4. Add message, SHOULD stay at bottom
    chatLog->addMessage("World");
    waitForRender(spy);

    QVERIFY(qAbs(vScroll->value() - vScroll->maximum()) <= 1);
}

void TestChatWidget::testSearch()
{
    // Use a fresh widget for this test to ensure clean state
    chatLog = std::make_unique<MockChatLog>(500);
    chatWidget = std::make_unique<MockChatWidget>(*chatLog, *idHandler, nullptr, *documentCache,
                                                  *smileyPack, *settings, *style, *messageBoxManager);
    chatWidget->show();
    chatWidget->resize(400, 300);

    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);
    waitForRender(spy);

    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    const int initialValue = vScroll->value();

    // Search for "Message 250"
    ParameterSearch params;
    params.period = PeriodSearch::WithTheFirst;
    chatWidget->startSearch("Message 250", params);
    waitForRender(spy);

    // The scrollbar should have moved significantly away from the bottom
    QVERIFY(vScroll->value() < initialValue - 100);

    // Test failing search
    QSignalSpy failSpy(chatWidget.get(), &ChatWidget::messageNotFoundShow);
    chatWidget->startSearch("Non-existent message", params);
    QVERIFY(failSpy.count() > 0 || failSpy.wait(500));
}

void TestChatWidget::testSelection()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);
    waitForRender(spy);

    chatWidget->selectAll();
    const QString selected = chatWidget->getSelectedText();

    // It should contain some messages
    QVERIFY(!selected.isEmpty());
    QVERIFY(selected.contains("Message"));

    chatWidget->clearSelection();
    QVERIFY(chatWidget->getSelectedText().isEmpty());
}

void TestChatWidget::testTypingNotification()
{
    // Initial state: typing notification might not exist yet
    chatWidget->setTypingNotificationVisible(true);
    chatWidget->setTypingNotificationName("User A");

    // Verify it doesn't crash and that the widget is still responsive.
    chatWidget->setTypingNotificationVisible(false);
    QVERIFY(true);
}

void TestChatWidget::testJumpToDate()
{
    // Use a fresh widget for this test
    chatLog = std::make_unique<MockChatLog>(500);
    chatWidget = std::make_unique<MockChatWidget>(*chatLog, *idHandler, nullptr, *documentCache,
                                                  *smileyPack, *settings, *style, *messageBoxManager);
    chatWidget->show();
    chatWidget->resize(400, 300);

    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);
    waitForRender(spy);

    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    const int initialValue = vScroll->value();

    // Jump to 2020-01-26 (around message 250)
    const QDate targetDate(2020, 1, 26);
    chatWidget->jumpToDate(targetDate);
    waitForRender(spy);

    // The scrollbar should have moved significantly away from the bottom
    QVERIFY(vScroll->value() < initialValue - 100);
}

void TestChatWidget::testClear()
{
    const QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    QVERIFY(!chatWidget->isEmpty());
    chatWidget->clear();

    QVERIFY(chatWidget->isEmpty());
}

void TestChatWidget::testSystemMessage()
{
    // Use a fresh widget with a small log
    chatLog = std::make_unique<MockChatLog>(20);
    chatWidget = std::make_unique<MockChatWidget>(*chatLog, *idHandler, nullptr, *documentCache,
                                                  *smileyPack, *settings, *style, *messageBoxManager);
    chatWidget->show();
    chatWidget->resize(400, 300);

    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);
    waitForRender(spy);

    SystemMessage sysMsg;
    sysMsg.messageType = SystemMessageType::peerNameChanged;
    sysMsg.timestamp = QDateTime::currentDateTime();
    sysMsg.args[0] = "User A";
    sysMsg.args[1] = "User B";

    chatLog->addSystemMessage(sysMsg);
    waitForRender(spy);

    // Should be visible at the bottom
    QScrollBar* vScroll = chatWidget->verticalScrollBar();
    QVERIFY(qAbs(vScroll->value() - vScroll->maximum()) <= 1);
}

void TestChatWidget::testScrollStress()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    // Add many messages to the existing log to stress virtualization
    for (int i = 0; i < 1000; ++i) {
        chatLog->addMessage(QString("Stress Message %1").arg(i));
    }
    waitForRender(spy);

    QScrollBar* vScroll = chatWidget->verticalScrollBar();

    auto scrollAndWait = [&](int value) {
        if (vScroll->value() == value)
            return;
        spy.clear();
        vScroll->setValue(value);
        waitForRender(spy);
    };

    // 1. Scroll up
    scrollAndWait(vScroll->minimum());
    QVERIFY(!chatWidget->isEmpty());

    // 2. Rapidly scroll down by chunks
    for (int i = 0; i < 5; ++i) {
        const int max = vScroll->maximum();
        scrollAndWait(max);

        QVERIFY(!chatWidget->isEmpty());
    }

    // 3. Jump to the middle and trigger pagination
    chatWidget->jumpToIdx(ChatLogIdx(500));
    waitForRender(spy);

    scrollAndWait(vScroll->maximum());
    scrollAndWait(vScroll->minimum());

    QVERIFY(!chatWidget->isEmpty());
}

void TestChatWidget::testResizeAndInterrupt()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);
    waitForRender(spy);

    // 1. Start a heavy render by jumping to a new location
    chatWidget->jumpToIdx(ChatLogIdx(500));

    // 2. Immediately resize while the worker is likely running
    chatWidget->resize(600, 400);
    chatWidget->resize(200, 600);

    // 3. Immediately clear while worker is running
    chatWidget->clear();

    waitForRender(spy);

    QVERIFY(chatWidget->isEmpty());
    QCOMPARE(chatWidget->verticalScrollBar()->maximum(), 0);

    // 4. Re-populate and ensure it still works
    chatWidget->jumpToIdx(ChatLogIdx(0));
    waitForRender(spy);
    QApplication::processEvents();

    QVERIFY(!chatWidget->isEmpty());
    // The scrollbar should be back to AsNeeded policy (not AlwaysOff)
    QVERIFY(chatWidget->verticalScrollBarPolicy() == Qt::ScrollBarAsNeeded);
}

void TestChatWidget::testClearPreservesFileTransfers()
{
    QSignalSpy spy(chatWidget.get(), &ChatWidget::renderFinished);

    // Add a normal message
    chatLog->addMessage("Normal message");
    waitForRender(spy);

    // Add a "file transfer" message
    chatLog->addMessage("File transfer message");
    waitForRender(spy);

    // Set the mock state: the second message is an "active transfer"
    chatWidget->mockActiveTransfer = true;

    QVERIFY(!chatWidget->isEmpty());

    chatWidget->clear();
    waitForRender(spy);

    // It should NOT be empty because we mocked an active transfer
    QVERIFY(!chatWidget->isEmpty());

    // Reset mock state and clear again
    chatWidget->mockActiveTransfer = false;
    chatWidget->clear();
    waitForRender(spy);

    // Now it should be empty
    QVERIFY(chatWidget->isEmpty());
}

QTEST_MAIN(TestChatWidget)
#include "chatwidget_test.moc"
