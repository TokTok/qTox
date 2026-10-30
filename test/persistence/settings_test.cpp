/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/persistence/settings.h"

#include "src/persistence/paths.h"
#include "src/widget/tool/imessageboxmanager.h"

#include <QDebug>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>

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
    bool askQuestion(const QString& title, const QString& msg, bool defaultAns, bool warning,
                     bool yesno) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = defaultAns;
        std::ignore = warning;
        std::ignore = yesno;
        return true;
    }
    bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                     const QString& button2, bool defaultAns, bool warning) override
    {
        std::ignore = title;
        std::ignore = msg;
        std::ignore = button1;
        std::ignore = button2;
        std::ignore = defaultAns;
        std::ignore = warning;
        return true;
    }
    void confirmExecutableOpen(const QFileInfo& file) override
    {
        std::ignore = file;
    }
};

MockMessageBoxManager::~MockMessageBoxManager() = default;

class TestSettings : public QObject
{
    Q_OBJECT

private slots:
    void testAutoSaveGlobal();
    void testAutoSaveDebounce();
    void testLogSettings();
};

void TestSettings::testAutoSaveGlobal()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    MockMessageBoxManager messageBoxManager;
    Settings settings(messageBoxManager, Paths::Portable::Portable);
    settings.getPaths().setPortablePath(tempDir.path());

    settings.setSaveTimerInterval(0);
    QVERIFY(!settings.getEnableDebug());

    settings.setEnableDebug(true);

    // Wait for the event loop to process the QTimer event in Settings::setVal
    QTest::qWait(250);

    // Block until the background QThread finishes writing to disk
    settings.sync();

    const QString filePath = settings.getPaths().getSettingsDirPath() + "qtox.ini";
    QVERIFY(QFile::exists(filePath));

    QSettings s(filePath, QSettings::IniFormat);
    s.beginGroup("Advanced");
    QVERIFY(s.value("enableDebug", false).toBool());
}

void TestSettings::testAutoSaveDebounce()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    MockMessageBoxManager messageBoxManager;
    Settings settings(messageBoxManager, Paths::Portable::Portable);
    settings.getPaths().setPortablePath(tempDir.path());

    // Use a small but measurable interval
    settings.setSaveTimerInterval(500);

    const QString filePath = settings.getPaths().getSettingsDirPath() + "qtox.ini";
    QFile::remove(filePath); // Ensure clean state
    QVERIFY(!QFile::exists(filePath));

    // First change starts the timer (500ms)
    settings.setEnableDebug(true);

    // Wait 250ms - timer shouldn't have fired yet
    QTest::qWait(250);
    settings.sync();                   // Wait for any *potential* I/O to finish
    QVERIFY(!QFile::exists(filePath)); // Verify no save happened yet

    // Second change should reset the timer back to 500ms
    settings.setEnableIPv6(false);

    // Wait another 250ms - a total of 500ms since the first change,
    // but only 250ms since the second change.
    // If it didn't reset, it would have fired by now.
    QTest::qWait(250);
    settings.sync();
    QVERIFY(!QFile::exists(filePath)); // Still shouldn't exist because it was reset

    // Wait 300ms more - now it has been 550ms since the *second* change,
    // so the timer must have fired and saved the file.
    QTest::qWait(300);
    settings.sync();
    QVERIFY(QFile::exists(filePath));
}

void TestSettings::testLogSettings()
{
    const QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    MockMessageBoxManager messageBoxManager;
    Settings settings(messageBoxManager, Paths::Portable::Portable);
    settings.getPaths().setPortablePath(tempDir.path());
    settings.setSaveTimerInterval(0);

    QCOMPARE(settings.getToxcoreLogLevel(), Settings::ToxcoreLogLevel::Debug);
    QVERIFY(settings.getLogToFile());

    settings.setToxcoreLogLevel(Settings::ToxcoreLogLevel::Trace);
    settings.setLogToFile(false);
    QTest::qWait(250);
    settings.sync();

    QSettings s(settings.getPaths().getSettingsDirPath() + "qtox.ini", QSettings::IniFormat);
    s.beginGroup("Advanced");
    QCOMPARE(s.value("toxcoreLogLevel").toInt(), static_cast<int>(Settings::ToxcoreLogLevel::Trace));
    QVERIFY(!s.value("logToFile", true).toBool());
}

QTEST_MAIN(TestSettings)
#include "settings_test.moc"
