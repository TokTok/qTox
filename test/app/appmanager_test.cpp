/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/appmanager.h"

#include <QEventLoop>
#include <QTimer>
#include <QtTest/QtTest>

class TestAppManager : public QObject
{
    Q_OBJECT
private slots:
    void terminateLeavesLoopBeforeMainLoop();
};

void TestAppManager::terminateLeavesLoopBeforeMainLoop()
{
    // Like the login screen, which runs before the main event loop.
    QEventLoop loop;
    QTimer::singleShot(0, &AppManager::terminate);
    QTimer::singleShot(1000, &loop, [&loop] { loop.exit(1); });
    QCOMPARE(loop.exec(), 0);
}

QTEST_GUILESS_MAIN(TestAppManager)
#include "appmanager_test.moc"
