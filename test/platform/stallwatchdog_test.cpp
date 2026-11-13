/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/platform/stallwatchdog.h"

#include <QStringList>
#include <QThread>
#include <QtTest/QtTest>

#include <mutex>
#include <tuple>
#include <utility>

namespace {
constexpr std::chrono::milliseconds stall{300};

std::mutex g_logMutex;
QStringList g_log;

void logToList(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    std::ignore = type;
    std::ignore = context;
    const std::scoped_lock<std::mutex> lock(g_logMutex);
    g_log.append(msg);
}

QStringList takeLog()
{
    const std::scoped_lock<std::mutex> lock(g_logMutex);
    return std::exchange(g_log, {});
}
} // namespace

class TestStallWatchdog : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();
    void silentWhileRunning();
    void logsStallWithStack();

private:
    QtMessageHandler previousHandler{nullptr};
};

void TestStallWatchdog::init()
{
    takeLog();
    previousHandler = qInstallMessageHandler(logToList);
}

void TestStallWatchdog::cleanup()
{
    qInstallMessageHandler(previousHandler);
}

void TestStallWatchdog::silentWhileRunning()
{
    const StallWatchdog watchdog{stall};
    QTest::qWait(stall.count() * 4);
    QCOMPARE(takeLog(), QStringList{});
}

void TestStallWatchdog::logsStallWithStack()
{
    {
        const StallWatchdog watchdog{stall};
        QThread::msleep(stall.count() * 3);
        QTest::qWait(stall.count());
    }
    const QStringList log = takeLog();

    QCOMPARE(log.filter("Event loop stalled for ").size(), 1);
    QVERIFY(log.first().startsWith("Event loop stalled for "));
    QVERIFY(log.last().startsWith("Event loop ran again after "));
#if defined(__GLIBC__) || defined(Q_OS_MACOS)
    // The stalled thread's stack: msleep() waits in nanosleep().
    QVERIFY2(!log.filter("nanosleep").isEmpty(), qPrintable(log.join('\n')));
#endif
}

QTEST_GUILESS_MAIN(TestStallWatchdog)
#include "stallwatchdog_test.moc"
