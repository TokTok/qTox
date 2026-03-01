/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "src/model/notificationgenerator.h"

#include "mock/mockconferencequery.h"
#include "mock/mockcoreidhandler.h"
#include "src/friendlist.h"

#include <QObject>
#include <QtTest/QtTest>

namespace {
class MockNotificationSettings : public INotificationSettings
{
    bool getNotify() const override
    {
        return true;
    }

    void setNotify(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getShowWindow() const override
    {
        return true;
    }
    void setShowWindow(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getDesktopNotify() const override
    {
        return true;
    }
    void setDesktopNotify(bool enabled) override
    {
        std::ignore = enabled;
    }

    bool getNotifySystemBackend() const override
    {
        return true;
    }
    void setNotifySystemBackend(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getNotifySound() const override
    {
        return true;
    }
    void setNotifySound(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getNotifyHide() const override
    {
        return notifyHide;
    }
    void setNotifyHide(bool newValue) override
    {
        notifyHide = newValue;
    }

    bool getBusySound() const override
    {
        return true;
    }
    void setBusySound(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getConferenceAlwaysNotify() const override
    {
        return true;
    }
    void setConferenceAlwaysNotify(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getHidePostNullSuffix() const override
    {
        return true;
    }
    void setHidePostNullSuffix(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getTypingNotification() const override
    {
        return true;
    }
    void setTypingNotification(bool newValue) override
    {
        std::ignore = newValue;
    }

    bool getStatusChangeNotificationEnabled() const override
    {
        return true;
    }
    void setStatusChangeNotificationEnabled(bool newValue) override
    {
        std::ignore = newValue;
    }

private:
    bool notifyHide = false;
};

} // namespace

class TestNotificationGenerator : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void testSingleFriendMessage();
    void testMultipleFriendMessages();
    void testNotificationClear();
    void testConferenceMessage();
    void testMultipleConferenceMessages();
    void testMultipleFriendSourceMessages();
    void testMultipleConferenceSourceMessages();
    void testMixedSourceMessages();
    void testFileTransfer();
    void testFileTransferAfterMessage();
    void testConferenceInvitation();
    void testConferenceInviteUncounted();
    void testFriendRequest();
    void testFriendRequestUncounted();
    void testSimpleFriendMessage();
    void testSimpleFileTransfer();
    void testSimpleConferenceMessage();
    void testSimpleFriendRequest();
    void testSimpleConferenceInvite();
    void testSimpleMessageToggle();

private:
    std::unique_ptr<INotificationSettings> notificationSettings;
    std::unique_ptr<NotificationGenerator> notificationGenerator;
    std::unique_ptr<MockConferenceQuery> conferenceQuery;
    std::unique_ptr<MockCoreIdHandler> coreIdHandler;
    std::unique_ptr<FriendList> friendList;
};

void TestNotificationGenerator::init()
{
    friendList = std::make_unique<FriendList>();
    notificationSettings = std::make_unique<MockNotificationSettings>();
    notificationGenerator = std::make_unique<NotificationGenerator>(*notificationSettings, nullptr);
    conferenceQuery = std::make_unique<MockConferenceQuery>();
    coreIdHandler = std::make_unique<MockCoreIdHandler>();
}

void TestNotificationGenerator::testSingleFriendMessage()
{
    Friend f(0, ToxPk());
    f.setName("friendName");
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test");
    QCOMPARE(notificationData.title, "friendName");
    QCOMPARE(notificationData.message, "test");
}

void TestNotificationGenerator::testMultipleFriendMessages()
{
    Friend f(0, ToxPk());
    f.setName("friendName");
    notificationGenerator->friendMessageNotification(&f, "test");
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");
    QCOMPARE(notificationData.title, "friendName");
    QCOMPARE(notificationData.message, "test2");

    notificationData = notificationGenerator->friendMessageNotification(&f, "test3");
    QCOMPARE(notificationData.title, "friendName");
    QCOMPARE(notificationData.message, "test3");
}

void TestNotificationGenerator::testNotificationClear()
{
    Friend f(0, ToxPk());
    f.setName("friendName");

    notificationGenerator->friendMessageNotification(&f, "test");

    // On notification clear we shouldn't see a notification count from the friend
    notificationGenerator->onNotificationActivated();

    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");
    QCOMPARE(notificationData.title, "friendName");
    QCOMPARE(notificationData.message, "test2");
}

void TestNotificationGenerator::testConferenceMessage()
{
    Conference g(0, ConferenceId(nullptr), "conferenceName", false, "selfName", *conferenceQuery,
                 *coreIdHandler, *friendList);
    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender, "test");
    QCOMPARE(notificationData.title, "conferenceName");
    QCOMPARE(notificationData.message, "sender1: test");
}

void TestNotificationGenerator::testMultipleConferenceMessages()
{
    Conference g(0, ConferenceId(nullptr), "conferenceName", false, "selfName", *conferenceQuery,
                 *coreIdHandler, *friendList);

    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    auto sender2 = conferenceQuery->getConferencePeerPk(0, 1);
    g.updateUsername(sender2, "sender2");

    notificationGenerator->conferenceMessageNotification(&g, sender, "test1");

    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender2, "test2");
    QCOMPARE(notificationData.title, "conferenceName");
    QCOMPARE(notificationData.message, "sender2: test2");
}

void TestNotificationGenerator::testMultipleFriendSourceMessages()
{
    Friend f(0, ToxPk());
    f.setName("friend1");

    Friend f2(1, ToxPk());
    f2.setName("friend2");

    notificationGenerator->friendMessageNotification(&f, "test1");
    auto notificationData = notificationGenerator->friendMessageNotification(&f2, "test2");

    QCOMPARE(notificationData.title, "friend2");
    QCOMPARE(notificationData.message, "test2");
}

void TestNotificationGenerator::testMultipleConferenceSourceMessages()
{
    Conference g1(0, ConferenceId(QByteArray(32, 0)), "conferenceName1", false, "selfName",
                  *conferenceQuery, *coreIdHandler, *friendList);
    Conference g2(1, ConferenceId(QByteArray(32, 1)), "conferenceName2", false, "selfName",
                  *conferenceQuery, *coreIdHandler, *friendList);

    auto sender_g1 = conferenceQuery->getConferencePeerPk(0, 1);
    g1.updateUsername(sender_g1, "sender1");

    auto sender_g2 = conferenceQuery->getConferencePeerPk(1, 1);
    g2.updateUsername(sender_g2, "sender1");

    notificationGenerator->conferenceMessageNotification(&g1, sender_g1, "test1");
    auto notificationData =
        notificationGenerator->conferenceMessageNotification(&g2, sender_g2, "test1");

    QCOMPARE(notificationData.title, "conferenceName2");
    QCOMPARE(notificationData.message, "sender1: test1");
}

void TestNotificationGenerator::testMixedSourceMessages()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    Conference g(0, ConferenceId(QByteArray(32, 0)), "conference", false, "selfName",
                 *conferenceQuery, *coreIdHandler, *friendList);

    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    notificationGenerator->friendMessageNotification(&f, "test1");
    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender, "test2");

    QCOMPARE("conference", notificationData.title);
    QCOMPARE("sender1: test2", notificationData.message);

    notificationData = notificationGenerator->fileTransferNotification(&f, "file", 0);
    QCOMPARE("friend - file transfer", notificationData.title);
    QCOMPARE("file (0B)", notificationData.message);
}

void TestNotificationGenerator::testFileTransfer()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    auto notificationData =
        notificationGenerator->fileTransferNotification(&f, "file", 5 * 1024 * 1024 /* 5MB */);

    QCOMPARE(notificationData.title, "friend - file transfer");
    QCOMPARE(notificationData.message, "file (5.00MiB)");
}

void TestNotificationGenerator::testFileTransferAfterMessage()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationGenerator->friendMessageNotification(&f, "test1");
    auto notificationData =
        notificationGenerator->fileTransferNotification(&f, "file", 5 * 1024 * 1024 /* 5MB */);

    QCOMPARE(notificationData.title, "friend - file transfer");
    QCOMPARE(notificationData.message, "file (5.00MiB)");
}

void TestNotificationGenerator::testConferenceInvitation()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    auto notificationData = notificationGenerator->conferenceInvitationNotification(&f);

    QCOMPARE(notificationData.title, "friend invites you to join a conference.");
    QCOMPARE(notificationData.message, "");
}

void TestNotificationGenerator::testConferenceInviteUncounted()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationGenerator->friendMessageNotification(&f, "test");
    notificationGenerator->conferenceInvitationNotification(&f);
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");

    QCOMPARE(notificationData.title, "friend");
    QCOMPARE(notificationData.message, "test2");
}

void TestNotificationGenerator::testFriendRequest()
{
    const ToxPk sender(QByteArray(32, 0));

    auto notificationData = notificationGenerator->friendRequestNotification(sender, "request");

    QCOMPARE(notificationData.title,
             "Friend request received from "
             "0000000000000000000000000000000000000000000000000000000000000000");
    QCOMPARE(notificationData.message, "request");
}

void TestNotificationGenerator::testFriendRequestUncounted()
{
    Friend f(0, ToxPk());
    f.setName("friend");
    const ToxPk sender(QByteArray(32, 0));

    notificationGenerator->friendMessageNotification(&f, "test");
    notificationGenerator->friendRequestNotification(sender, "request");
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");

    QCOMPARE(notificationData.title, "friend");
    QCOMPARE(notificationData.message, "test2");
}

void TestNotificationGenerator::testSimpleFriendMessage()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test");

    QCOMPARE(notificationData.title, "New message");
    QCOMPARE(notificationData.message, "");
}

void TestNotificationGenerator::testSimpleFileTransfer()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->fileTransferNotification(&f, "file", 0);

    QCOMPARE(notificationData.title, "Incoming file transfer");
    QCOMPARE(notificationData.message, "");
}

void TestNotificationGenerator::testSimpleConferenceMessage()
{
    Conference g(0, ConferenceId(nullptr), "conferenceName", false, "selfName", *conferenceQuery,
                 *coreIdHandler, *friendList);
    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender, "test");
    QCOMPARE(notificationData.title, "New conference message");
    QCOMPARE(notificationData.message, "");
}

void TestNotificationGenerator::testSimpleFriendRequest()
{
    const ToxPk sender(QByteArray(32, 0));

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->friendRequestNotification(sender, "request");

    QCOMPARE(notificationData.title, "Friend request received");
    QCOMPARE(notificationData.message, "");
}

void TestNotificationGenerator::testSimpleConferenceInvite()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);
    auto notificationData = notificationGenerator->conferenceInvitationNotification(&f);

    QCOMPARE(notificationData.title, "Conference invite received");
    QCOMPARE(notificationData.message, "");
}

void TestNotificationGenerator::testSimpleMessageToggle()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);

    notificationGenerator->friendMessageNotification(&f, "test");

    notificationSettings->setNotifyHide(false);

    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");

    QCOMPARE(notificationData.title, "friend");
    QCOMPARE(notificationData.message, "test2");
}

QTEST_GUILESS_MAIN(TestNotificationGenerator)
#include "notificationgenerator_test.moc"
