/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/chatlog/content/text.h"

#include "src/chatlog/documentcache.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/style.h"
#include "src/widget/tool/imessageboxmanager.h"

#include <QApplication>
#include <QObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

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
    bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                     bool warning = true, bool yesno = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = warning;
        std::ignore = yesno;
        return defaultAns;
    }
    bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                     const QString& button2, bool defaultAns = false, bool warning = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = button1;
        std::ignore = button2;
        std::ignore = warning;
        return defaultAns;
    }
    IMessageBoxManager::Answer askQuestion(const QString& title, const QString& msg,
                                           const QString& button1, const QString& button2,
                                           const QString& button3, bool defaultAns = false,
                                           bool warning = true) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = button1;
        std::ignore = button2;
        std::ignore = button3;
        std::ignore = warning;
        return defaultAns ? IMessageBoxManager::Answer::Accepted
                          : IMessageBoxManager::Answer::Rejected;
    }
    void confirmExecutableOpen(const QFileInfo& file) override
    {
        std::ignore = file;
    }
};

MockMessageBoxManager::~MockMessageBoxManager() = default;

class TestText : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void testGetLinkAtNullDoc();

private:
    std::unique_ptr<QTemporaryDir> tempHome;
    std::unique_ptr<MockMessageBoxManager> messageBoxManager;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<Style> style;
    std::unique_ptr<SmileyPack> smileyPack;
    std::unique_ptr<DocumentCache> documentCache;
};

void TestText::init()
{
    tempHome = std::make_unique<QTemporaryDir>();
    qputenv("HOME", tempHome->path().toUtf8());
    qputenv("XDG_CONFIG_HOME", tempHome->path().toUtf8());

    messageBoxManager = std::make_unique<MockMessageBoxManager>();
    settings = std::make_unique<Settings>(*messageBoxManager);
    style = std::make_unique<Style>();
    smileyPack = std::make_unique<SmileyPack>(*settings);
    documentCache = std::make_unique<DocumentCache>(*smileyPack, *settings);
}

void TestText::cleanup()
{
    documentCache.reset();
    smileyPack.reset();
    style.reset();
    settings.reset();
    messageBoxManager.reset();
    tempHome.reset();
}

void TestText::testGetLinkAtNullDoc()
{
    Text text(*documentCache, *settings, *style, QStringLiteral("some text"));

    // Ensure document is not in memory
    text.visibilityChanged(false);

    // This should not crash
    const QString link = text.getLinkAt(QPointF(0, 0));
    QVERIFY(link.isEmpty());
}

QTEST_MAIN(TestText)
#include "text_test.moc"
