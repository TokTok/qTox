/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/core/core.h"

#include "mock/mockbootstraplistgenerator.h"
#include "mock/mockcoresettings.h"
#include "src/core/icoresettings.h"
#include "src/model/ibootstraplistgenerator.h"

#include <QSignalSpy>
#include <QtGlobal>
#include <QtTest/QtTest>

#include <memory>

Q_DECLARE_METATYPE(QList<DhtServer>)
Q_DECLARE_METATYPE(ToxPk)
Q_DECLARE_METATYPE(uint32_t)
Q_DECLARE_METATYPE(Status::Status)

class TestCore : public QObject
{
    Q_OBJECT
public:
    TestCore()
    {
        qRegisterMetaType<ToxPk>("ToxPk");
        qRegisterMetaType<uint32_t>("uint32_t");
        qRegisterMetaType<Status::Status>("Status::Status");
    }

private slots:
    void startup_with_invalid_socks5_proxy();
    void startup_with_invalid_http_proxy();
    void bootstrap();
    void make_friends();
    void change_name();
    void change_status_message();
    void change_status();

private:
    /* Test Variables */
    MockSettings settings;
    ToxCorePtr alice;
    ToxCorePtr bob;
    MockBootstrapListGenerator alicesNodes{};
    MockBootstrapListGenerator bobsNodes{};
};

namespace {
const int bootstrap_timeout = 20000;
const int connected_message_wait = 5000;

void bootstrapToxes(Core& alice, MockBootstrapListGenerator& alicesNodes, Core& bob,
                    MockBootstrapListGenerator& bobsNodes)
{
    alicesNodes.setBootstrapNodes(MockBootstrapListGenerator::makeListFromSelf(bob));
    bobsNodes.setBootstrapNodes(MockBootstrapListGenerator::makeListFromSelf(alice));

    QSignalSpy spyAlice(&alice, &Core::connected);
    QSignalSpy spyBob(&bob, &Core::connected);

    alice.start();
    bob.start();

    QTRY_VERIFY_WITH_TIMEOUT(spyAlice.count() == 1 && spyBob.count() == 1, bootstrap_timeout);
}
} // namespace

void TestCore::startup_with_invalid_socks5_proxy()
{
    settings.setProxyAddr("Test::haha:::nope");
    settings.setProxyPort(9985);
    settings.setProxyType(MockSettings::ProxyType::ptSOCKS5);

    auto [core, err] = Core::makeToxCore({}, settings, alicesNodes);

    QVERIFY(!core);
    QCOMPARE(err, Core::ToxCoreErrors::BAD_PROXY);
}

void TestCore::startup_with_invalid_http_proxy()
{
    settings.setProxyAddr("Test::haha:::nope");
    settings.setProxyPort(9985);
    settings.setProxyType(MockSettings::ProxyType::ptHTTP);

    auto [core, err] = Core::makeToxCore({}, settings, alicesNodes);

    QVERIFY(!core);
    QCOMPARE(err, Core::ToxCoreErrors::BAD_PROXY);
}

void TestCore::bootstrap()
{
    // No proxy
    settings.setProxyAddr("");
    settings.setProxyPort(0);
    settings.setProxyType(MockSettings::ProxyType::ptNone);

    auto [aliceCore, aliceErr] = Core::makeToxCore({}, settings, alicesNodes);
    auto [bobCore, bobErr] = Core::makeToxCore({}, settings, bobsNodes);
    QVERIFY(aliceCore != nullptr);
    QVERIFY(bobCore != nullptr);

    alice = std::move(aliceCore);
    bob = std::move(bobCore);

    bootstrapToxes(*alice, alicesNodes, *bob, bobsNodes);
}

void TestCore::make_friends()
{
    // Make a friend request from alice to bob
    const QLatin1String friendMsg{"Test Invite Message"};

    QSignalSpy spyBobFriendMsg(bob.get(), &Core::friendRequestReceived);
    QSignalSpy spyAliceFriendMsg(alice.get(), &Core::requestSent);
    alice->requestFriendship(bob->getSelfId(), friendMsg);

    // Wait for friend message to be received
    QTRY_VERIFY_WITH_TIMEOUT(spyBobFriendMsg.count() == 1 && spyAliceFriendMsg.count() == 1,
                             connected_message_wait);

    // Check for expected signal content
    QVERIFY(qvariant_cast<ToxPk>(spyBobFriendMsg[0][0]) == alice->getSelfPublicKey());
    QVERIFY(spyBobFriendMsg[0][1].toString() == friendMsg);

    QVERIFY(qvariant_cast<ToxPk>(spyAliceFriendMsg[0][0]) == bob->getSelfPublicKey());
    QVERIFY(spyAliceFriendMsg[0][1].toString() == friendMsg);

    // Let Bob accept the friend request from Alice
    bob->acceptFriendRequest(alice->getSelfPublicKey());

    // Wait until both see each other
    QSignalSpy spyAliceFriendOnline(alice.get(), &Core::friendStatusChanged);
    QSignalSpy spyBobFriendOnline(bob.get(), &Core::friendStatusChanged);

    QTRY_VERIFY_WITH_TIMEOUT(spyAliceFriendOnline.count() >= 1 && spyBobFriendOnline.count() >= 1,
                             connected_message_wait);

    // Check for expected signal content
    QVERIFY(spyAliceFriendOnline[0][0].toInt() == static_cast<int>(Status::Status::Online));
    QVERIFY(spyBobFriendOnline[0][0].toInt() == static_cast<int>(Status::Status::Online));
}

void TestCore::change_name()
{
    // Change the name of Alice to "Alice"
    const QLatin1String aliceName{"Alice"};

    QSignalSpy aliceSaveRequest(alice.get(), &Core::saveRequest);
    QSignalSpy aliceUsernameChanged(alice.get(), &Core::usernameSet);
    QSignalSpy bobUsernameChangeReceived(bob.get(), &Core::friendUsernameChanged);

    alice->setUsername(aliceName);

    QTRY_VERIFY_WITH_TIMEOUT(aliceSaveRequest.count() == 1, bootstrap_timeout);
    QTRY_VERIFY_WITH_TIMEOUT(aliceUsernameChanged.count() == 1
                                 && aliceUsernameChanged[0][0].toString() == aliceName,
                             bootstrap_timeout);

    QTRY_VERIFY_WITH_TIMEOUT(bobUsernameChangeReceived.count() == 1
                                 && bobUsernameChangeReceived[0][1].toString() == aliceName,
                             bootstrap_timeout);

    // Setting the username again to the same value should NOT trigger any signals
    alice->setUsername(aliceName);

    // Need to sleep here, because we're testing that these don't increase based on
    // Alice re-setting her username, so QTRY_VERIFY_WITH_TIMEOUT would return immediately.
    QTest::qSleep(connected_message_wait);
    QVERIFY(aliceSaveRequest.count() == 1);
    QVERIFY(aliceUsernameChanged.count() == 1);
    QVERIFY(bobUsernameChangeReceived.count() == 1);
}

void TestCore::change_status_message()
{
    // Change the status message of Alice
    const QLatin1String aliceStatusMsg{"Testing a lot"};

    QSignalSpy aliceSaveRequest(alice.get(), &Core::saveRequest);
    QSignalSpy aliceStatusMsgChanged(alice.get(), &Core::statusMessageSet);
    QSignalSpy bobStatusMsgChangeReceived(bob.get(), &Core::friendStatusMessageChanged);

    alice->setStatusMessage(aliceStatusMsg);

    QTRY_VERIFY_WITH_TIMEOUT(aliceSaveRequest.count() == 1, connected_message_wait);
    QTRY_VERIFY_WITH_TIMEOUT(aliceStatusMsgChanged.count() == 1
                                 && aliceStatusMsgChanged[0][0].toString() == aliceStatusMsg,
                             connected_message_wait);

    QTRY_VERIFY_WITH_TIMEOUT(bobStatusMsgChangeReceived.count() == 1
                                 && bobStatusMsgChangeReceived[0][1].toString() == aliceStatusMsg,
                             connected_message_wait);

    // Setting the status message again to the same value should NOT trigger any signals
    alice->setStatusMessage(aliceStatusMsg);

    // Need to sleep here, because we're testing that these don't increase based on
    // Alice re-setting her status message, so QTRY_VERIFY_WITH_TIMEOUT would return immediately.
    QTest::qSleep(connected_message_wait);
    QVERIFY(aliceSaveRequest.count() == 1);
    QVERIFY(aliceStatusMsgChanged.count() == 1);
    QVERIFY(bobStatusMsgChangeReceived.count() == 1);
}

void TestCore::change_status()
{
    QSignalSpy aliceSaveRequest(alice.get(), &Core::saveRequest);
    QSignalSpy aliceStatusChanged(alice.get(), &Core::statusSet);
    QSignalSpy bobStatusChangeReceived(bob.get(), &Core::friendStatusChanged);

    alice->setStatus(Status::Status::Away);

    QTRY_VERIFY_WITH_TIMEOUT(aliceSaveRequest.count() == 1, connected_message_wait);
    QTRY_VERIFY_WITH_TIMEOUT(aliceStatusChanged.count() == 1
                                 && qvariant_cast<Status::Status>(aliceStatusChanged[0][0])
                                        == Status::Status::Away,
                             connected_message_wait);

    QTRY_VERIFY_WITH_TIMEOUT(bobStatusChangeReceived.count() == 1
                                 && qvariant_cast<Status::Status>(bobStatusChangeReceived[0][1])
                                        == Status::Status::Away,
                             connected_message_wait);

    // Setting the status message again to the same value again triggers all signals
    alice->setStatus(Status::Status::Away);

    // TODO(sudden6): Emitting these again odd and should probably be changed, lets codify it for now though
    QTRY_VERIFY_WITH_TIMEOUT(aliceSaveRequest.count() == 2, connected_message_wait);
    QTRY_VERIFY_WITH_TIMEOUT(aliceStatusChanged.count() == 2
                                 && qvariant_cast<Status::Status>(aliceStatusChanged[1][0])
                                        == Status::Status::Away,
                             connected_message_wait);

    // Need to sleep here, because we're testing that we don't get a new signal for the re-set but
    // unchanged status, so QTRY_VERIFY_WITH_TIMEOUT would return immediately.
    QTest::qSleep(connected_message_wait);
    QVERIFY(bobStatusChangeReceived.count() == 1);
}

QTEST_GUILESS_MAIN(TestCore)
#include "core_test.moc"
