/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2018-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/net/bootstrapnodeupdater.h"
#include "src/persistence/paths.h"

#include <QNetworkProxy>
#include <QSignalSpy>

#include <QtTest/QtTest>

// Needed to make this type known to Qt
Q_DECLARE_METATYPE(QList<DhtServer>)

class TestBootstrapNodesUpdater : public QObject
{
    Q_OBJECT
public:
    TestBootstrapNodesUpdater();
private slots:
    void testOnline();
    void testLocal();
};

TestBootstrapNodesUpdater::TestBootstrapNodesUpdater()
{
    if (qgetenv("HOME").isEmpty()) {
        qputenv("HOME", qgetenv("TEST_TMPDIR").append("/home"));
    }

    qRegisterMetaType<QList<DhtServer>>("QList<DhtServer>");
    // Contains the builtin nodes list
    Q_INIT_RESOURCE(res);
}

void TestBootstrapNodesUpdater::testOnline()
{
    QNetworkProxy proxy{QNetworkProxy::ProxyType::NoProxy};
    Paths paths{Paths::Portable::NonPortable};

    BootstrapNodeUpdater updater{proxy, paths};
    QSignalSpy spy(&updater, &BootstrapNodeUpdater::availableBootstrapNodes);

    updater.requestBootstrapNodes();

    spy.wait(10000); // increase wait time for sporadic CI failures with slow nodes server
    QCOMPARE(spy.count(), 1); // make sure the signal was emitted exactly one time
    QList<DhtServer> result = qvariant_cast<QList<DhtServer>>(spy.at(0).at(0));
    QVERIFY(result.size() > 0); // some data should be returned
}

void TestBootstrapNodesUpdater::testLocal()
{
    QList<DhtServer> defaultNodes = BootstrapNodeUpdater::loadDefaultBootstrapNodes();
    QVERIFY(defaultNodes.size() > 0);
}

QTEST_GUILESS_MAIN(TestBootstrapNodesUpdater)
#include "bsu_test.moc"
