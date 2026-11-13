/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/model/group.h"

#include "mock/mockcoreidhandler.h"
#include "mock/mockgroupquery.h"
#include "src/core/icoregroupmessagesender.h"
#include "src/friendlist.h"
#include "src/model/groupmessagedispatcher.h"
#include "src/persistence/iconferencesettings.h"
#include "util/interface.h"

#include <QList>
#include <QtTest/QtTest>

#include <tox/tox.h> // tox_max_message_length

namespace {
class RecordingGroupSender : public ICoreGroupMessageSender
{
public:
    void sendGroupAction(uint32_t groupNumber, const QString& message) override
    {
        std::ignore = groupNumber;
        std::ignore = message;
    }

    void sendGroupMessage(uint32_t groupNumber, const QString& message) override
    {
        std::ignore = groupNumber;
        std::ignore = message;
    }

    void sendGroupPrivateMessage(uint32_t groupNumber, const ToxPk& recipient,
                                 const QString& message) override
    {
        std::ignore = groupNumber;
        std::ignore = message;
        recipients.append(recipient);
    }

    void sendGroupPrivateAction(uint32_t groupNumber, const ToxPk& recipient, const QString& message) override
    {
        sendGroupPrivateMessage(groupNumber, recipient, message);
    }

    QList<ToxPk> recipients;
};

ToxPk peerKey(uint8_t first)
{
    uint8_t key[ToxPk::size] = {first};
    return ToxPk(key);
}
} // namespace

class NoBlocking : public QObject, public IConferenceSettings
{
    Q_OBJECT

public:
    QStringList getBlockList() const override
    {
        return {};
    }
    void setBlockList(const QStringList& blist) override
    {
        std::ignore = blist;
    }
    SIGNAL_IMPL(NoBlocking, blockListChanged, const QStringList& blist)

    bool getShowConferenceJoinLeaveMessages() const override
    {
        return false;
    }
    void setShowConferenceJoinLeaveMessages(bool newValue) override
    {
        std::ignore = newValue;
    }
    SIGNAL_IMPL(NoBlocking, showConferenceJoinLeaveMessagesChanged, bool show)
};

class TestGroup : public QObject
{
    Q_OBJECT

private slots:
    void privateMessageGoesToTheMembersKey();
};

/**
 * @brief Toxcore gives a new peer the lowest free peer number, so a number
 * qTox remembered can be someone else's by the time a message goes. Members
 * are known by key outside Core, and Core finds their number when it sends.
 */
void TestGroup::privateMessageGoesToTheMembersKey()
{
    const ToxPk alice = peerKey(1);
    MockGroupQuery query;
    MockCoreIdHandler idHandler;
    FriendList friendList;
    NoBlocking settings;
    RecordingGroupSender sender;
    Group group(0, GroupId{}, QStringLiteral("group"), QStringLiteral("self"), query, idHandler,
                friendList);
    MessageProcessor::SharedParams params(tox_max_message_length());
    GroupMessageDispatcher dispatcher(group, MessageProcessor(params), idHandler, sender, settings);

    group.onPeerJoin(alice, QStringLiteral("Alice"), GroupRole::User, Status::Status::Online);
    dispatcher.sendPrivateMessage(alice, false, QStringLiteral("psst"));

    QCOMPARE(sender.recipients, QList<ToxPk>{alice});
}

QTEST_GUILESS_MAIN(TestGroup)
#include "group_test.moc"
