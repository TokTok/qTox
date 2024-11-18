/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/friendlist.h"
#include "src/model/notificationgenerator.h"

#include "mock/mockcoreidhandler.h"
#include "mock/mockconferencequery.h"

#include <QObject>
#include <QtTest/QtTest>

namespace {
class MockNotificationSettings : public INotificationSettings
{
    virtual bool getNotify() const override
    {
        return true;
    }

    virtual void setNotify(bool newValue) override
    {
        std::ignore = newValue;
    }

    virtual bool getShowWindow() const override
    {
        return true;
    }
    virtual void setShowWindow(bool newValue) override
    {
        std::ignore = newValue;
    }

    virtual bool getDesktopNotify() const override
    {
        return true;
    }
    virtual void setDesktopNotify(bool enabled) override
    {
        std::ignore = enabled;
    }

    virtual bool getNotifySound() const override
    {
        return true;
    }
    virtual void setNotifySound(bool newValue) override
    {
        std::ignore = newValue;
    }

    virtual bool getNotifyHide() const override
    {
        return notifyHide;
    }
    virtual void setNotifyHide(bool newValue) override
    {
        notifyHide = newValue;
    }

    virtual bool getBusySound() const override
    {
        return true;
    }
    virtual void setBusySound(bool newValue) override
    {
        std::ignore = newValue;
    }

    virtual bool getConferenceAlwaysNotify() const override
    {
        return true;
    }
    virtual void setConferenceAlwaysNotify(bool newValue) override
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
    friendList.reset(new FriendList());
    notificationSettings.reset(new MockNotificationSettings());
    notificationGenerator.reset(new NotificationGenerator(*notificationSettings, nullptr));
    conferenceQuery.reset(new MockConferenceQuery());
    coreIdHandler.reset(new MockCoreIdHandler());
}

void TestNotificationGenerator::testSingleFriendMessage()
{
    Friend f(0, ToxPk());
    f.setName("friendName");
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test");
    QVERIFY(notificationData.title == "friendName");
    QVERIFY(notificationData.message == "test");
}

void TestNotificationGenerator::testMultipleFriendMessages()
{
    Friend f(0, ToxPk());
    f.setName("friendName");
    notificationGenerator->friendMessageNotification(&f, "test");
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");
    QVERIFY(notificationData.title == "2 message(s) from friendName");
    QVERIFY(notificationData.message == "test2");

    notificationData = notificationGenerator->friendMessageNotification(&f, "test3");
    QVERIFY(notificationData.title == "3 message(s) from friendName");
    QVERIFY(notificationData.message == "test3");
}

void TestNotificationGenerator::testNotificationClear()
{
    Friend f(0, ToxPk());
    f.setName("friendName");

    notificationGenerator->friendMessageNotification(&f, "test");

    // On notification clear we shouldn't see a notification count from the friend
    notificationGenerator->onNotificationActivated();

    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");
    QVERIFY(notificationData.title == "friendName");
    QVERIFY(notificationData.message == "test2");
}

void TestNotificationGenerator::testConferenceMessage()
{
    Conference g(0, ConferenceId(0), "conferenceName", false, "selfName", *conferenceQuery, *coreIdHandler, *friendList);
    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender, "test");
    QVERIFY(notificationData.title == "conferenceName");
    QVERIFY(notificationData.message == "sender1: test");
}

void TestNotificationGenerator::testMultipleConferenceMessages()
{
    Conference g(0, ConferenceId(0), "conferenceName", false, "selfName", *conferenceQuery, *coreIdHandler, *friendList);

    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    auto sender2 = conferenceQuery->getConferencePeerPk(0, 1);
    g.updateUsername(sender2, "sender2");

    notificationGenerator->conferenceMessageNotification(&g, sender, "test1");

    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender2, "test2");
    QVERIFY(notificationData.title == "2 message(s) from conferenceName");
    QVERIFY(notificationData.message == "sender2: test2");
}

void TestNotificationGenerator::testMultipleFriendSourceMessages()
{
    Friend f(0, ToxPk());
    f.setName("friend1");

    Friend f2(1, ToxPk());
    f2.setName("friend2");

    notificationGenerator->friendMessageNotification(&f, "test1");
    auto notificationData = notificationGenerator->friendMessageNotification(&f2, "test2");

    QVERIFY(notificationData.title == "2 message(s) from 2 chats");
    QVERIFY(notificationData.message == "friend1, friend2");
}

void TestNotificationGenerator::testMultipleConferenceSourceMessages()
{
    Conference g(0, ConferenceId(QByteArray(32, 0)), "conferenceName", false, "selfName", *conferenceQuery,
            *coreIdHandler, *friendList);
    Conference g2(1, ConferenceId(QByteArray(32, 1)), "conferenceName2", false, "selfName", *conferenceQuery,
             *coreIdHandler, *friendList);

    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    notificationGenerator->conferenceMessageNotification(&g, sender, "test1");
    auto notificationData = notificationGenerator->conferenceMessageNotification(&g2, sender, "test1");

    QVERIFY(notificationData.title == "2 message(s) from 2 chats");
    QVERIFY(notificationData.message == "conferenceName, conferenceName2");
}

void TestNotificationGenerator::testMixedSourceMessages()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    Conference g(0, ConferenceId(QByteArray(32, 0)), "conference", false, "selfName", *conferenceQuery, *coreIdHandler,
            *friendList);

    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    notificationGenerator->friendMessageNotification(&f, "test1");
    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender, "test2");

    QVERIFY(notificationData.title == "2 message(s) from 2 chats");
    QVERIFY(notificationData.message == "friend, conference");

    notificationData = notificationGenerator->fileTransferNotification(&f, "file", 0);
    QVERIFY(notificationData.title == "3 message(s) from 2 chats");
    QVERIFY(notificationData.message == "friend, conference");
}

void TestNotificationGenerator::testFileTransfer()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    auto notificationData =
        notificationGenerator->fileTransferNotification(&f, "file", 5 * 1024 * 1024 /* 5MB */);

    QVERIFY(notificationData.title == "friend - file transfer");
    QVERIFY(notificationData.message == "file (5.00MiB)");
}

void TestNotificationGenerator::testFileTransferAfterMessage()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationGenerator->friendMessageNotification(&f, "test1");
    auto notificationData =
        notificationGenerator->fileTransferNotification(&f, "file", 5 * 1024 * 1024 /* 5MB */);

    QVERIFY(notificationData.title == "2 message(s) from friend");
    QVERIFY(notificationData.message == "Incoming file transfer");
}

void TestNotificationGenerator::testConferenceInvitation()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    auto notificationData = notificationGenerator->conferenceInvitationNotification(&f);

    QVERIFY(notificationData.title == "friend invites you to join a conference.");
    QVERIFY(notificationData.message == "");
}

void TestNotificationGenerator::testConferenceInviteUncounted()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationGenerator->friendMessageNotification(&f, "test");
    notificationGenerator->conferenceInvitationNotification(&f);
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");

    QVERIFY(notificationData.title == "2 message(s) from friend");
    QVERIFY(notificationData.message == "test2");
}

void TestNotificationGenerator::testFriendRequest()
{
    ToxPk sender(QByteArray(32, 0));

    auto notificationData = notificationGenerator->friendRequestNotification(sender, "request");

    QVERIFY(notificationData.title == "Friend request received from 0000000000000000000000000000000000000000000000000000000000000000");
    QVERIFY(notificationData.message == "request");
}

void TestNotificationGenerator::testFriendRequestUncounted()
{
    Friend f(0, ToxPk());
    f.setName("friend");
    ToxPk sender(QByteArray(32, 0));

    notificationGenerator->friendMessageNotification(&f, "test");
    notificationGenerator->friendRequestNotification(sender, "request");
    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");

    QVERIFY(notificationData.title == "2 message(s) from friend");
    QVERIFY(notificationData.message == "test2");
}

void TestNotificationGenerator::testSimpleFriendMessage()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test");

    QVERIFY(notificationData.title == "New message");
    QVERIFY(notificationData.message == "");
}

void TestNotificationGenerator::testSimpleFileTransfer()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->fileTransferNotification(&f, "file", 0);

    QVERIFY(notificationData.title == "Incoming file transfer");
    QVERIFY(notificationData.message == "");
}

void TestNotificationGenerator::testSimpleConferenceMessage()
{
    Conference g(0, ConferenceId(0), "conferenceName", false, "selfName", *conferenceQuery, *coreIdHandler, *friendList);
    auto sender = conferenceQuery->getConferencePeerPk(0, 0);
    g.updateUsername(sender, "sender1");

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->conferenceMessageNotification(&g, sender, "test");
    QVERIFY(notificationData.title == "New conference message");
    QVERIFY(notificationData.message == "");
}

void TestNotificationGenerator::testSimpleFriendRequest()
{
    ToxPk sender(QByteArray(32, 0));

    notificationSettings->setNotifyHide(true);

    auto notificationData = notificationGenerator->friendRequestNotification(sender, "request");

    QVERIFY(notificationData.title == "Friend request received");
    QVERIFY(notificationData.message == "");
}

void TestNotificationGenerator::testSimpleConferenceInvite()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);
    auto notificationData = notificationGenerator->conferenceInvitationNotification(&f);

    QVERIFY(notificationData.title == "Conference invite received");
    QVERIFY(notificationData.message == "");
}

void TestNotificationGenerator::testSimpleMessageToggle()
{
    Friend f(0, ToxPk());
    f.setName("friend");

    notificationSettings->setNotifyHide(true);

    notificationGenerator->friendMessageNotification(&f, "test");

    notificationSettings->setNotifyHide(false);

    auto notificationData = notificationGenerator->friendMessageNotification(&f, "test2");

    QVERIFY(notificationData.title == "2 message(s) from friend");
    QVERIFY(notificationData.message == "test2");
}

QTEST_GUILESS_MAIN(TestNotificationGenerator)
#include "notificationgenerator_test.moc"
