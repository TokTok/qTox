/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/core/core.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/persistence/offlinemsgengine.h"

#include <QtTest/QtTest>

class TestOfflineMsgEngine : public QObject
{
    Q_OBJECT

private slots:
    void testReceiptBeforeMessage();
    void testReceiptAfterMessage();
    void testResendWorkflow();
    void testTypeCoordination();
    void testCallback();
};

namespace {
void completionFn(bool success)
{
    std::ignore = success;
}
} // namespace

void TestOfflineMsgEngine::testReceiptBeforeMessage()
{
    OfflineMsgEngine offlineMsgEngine;

    Message msg{false, QString(), QDateTime(), {}};

    const auto receipt = ReceiptNum(0);
    offlineMsgEngine.onReceiptReceived(receipt);
    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);

    const auto removedMessages = offlineMsgEngine.removeAllMessages();

    QVERIFY(removedMessages.empty());
}

void TestOfflineMsgEngine::testReceiptAfterMessage()
{
    OfflineMsgEngine offlineMsgEngine;

    const auto receipt = ReceiptNum(0);
    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);
    offlineMsgEngine.onReceiptReceived(receipt);

    const auto removedMessages = offlineMsgEngine.removeAllMessages();

    QVERIFY(removedMessages.empty());
}

void TestOfflineMsgEngine::testResendWorkflow()
{
    OfflineMsgEngine offlineMsgEngine;

    const auto receipt = ReceiptNum(0);
    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);
    auto messagesToResend = offlineMsgEngine.removeAllMessages();

    QCOMPARE(messagesToResend.size(), 1);

    offlineMsgEngine.addSentCoreMessage(receipt, Message(), completionFn);
    offlineMsgEngine.onReceiptReceived(receipt);

    messagesToResend = offlineMsgEngine.removeAllMessages();
    QVERIFY(messagesToResend.empty());

    const auto nullMsg = Message();
    auto msg2 = Message();
    auto msg3 = Message();
    msg2.content = "msg2";
    msg3.content = "msg3";
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(0), nullMsg, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(1), nullMsg, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(2), msg2, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(3), msg3, completionFn);

    offlineMsgEngine.onReceiptReceived(ReceiptNum(0));
    offlineMsgEngine.onReceiptReceived(ReceiptNum(1));
    offlineMsgEngine.onReceiptReceived(ReceiptNum(3));

    messagesToResend = offlineMsgEngine.removeAllMessages();
    QCOMPARE(messagesToResend.size(), 1);
    QCOMPARE(messagesToResend[0].message.content, "msg2");
}


void TestOfflineMsgEngine::testTypeCoordination()
{
    OfflineMsgEngine offlineMsgEngine;

    auto msg1 = Message();
    auto msg2 = Message();
    auto msg3 = Message();
    auto msg4 = Message();

    msg1.content = "msg1";
    msg2.content = "msg2";
    msg3.content = "msg3";
    msg4.content = "msg4";

    offlineMsgEngine.addSentCoreMessage(ReceiptNum(1), msg1, completionFn);
    offlineMsgEngine.addUnsentMessage(msg2, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(2), msg3, completionFn);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(3), msg4, completionFn);

    const auto messagesToResend = offlineMsgEngine.removeAllMessages();

    QCOMPARE(messagesToResend.size(), 4);
    QCOMPARE(messagesToResend[0].message.content, "msg1");
    QCOMPARE(messagesToResend[1].message.content, "msg2");
    QCOMPARE(messagesToResend[2].message.content, "msg3");
    QCOMPARE(messagesToResend[3].message.content, "msg4");
}

void TestOfflineMsgEngine::testCallback()
{
    OfflineMsgEngine offlineMsgEngine;

    size_t numCallbacks = 0;
    auto callback = [&numCallbacks](bool) { numCallbacks++; };
    Message msg{false, QString(), QDateTime(), {}};
    ReceiptNum receipt;

    offlineMsgEngine.addSentCoreMessage(ReceiptNum(1), Message(), callback);
    offlineMsgEngine.addSentCoreMessage(ReceiptNum(2), Message(), callback);

    offlineMsgEngine.onReceiptReceived(ReceiptNum(1));
    offlineMsgEngine.onReceiptReceived(ReceiptNum(2));

    QCOMPARE(numCallbacks, 2);
}

QTEST_GUILESS_MAIN(TestOfflineMsgEngine)
#include "offlinemsgengine_test.moc"
