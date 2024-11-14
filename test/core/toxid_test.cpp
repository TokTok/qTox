/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/core/toxid.h"

#include <QtTest/QtTest>
#include <QString>
#include <ctime>

const QString corrupted =
    QStringLiteral("A369865720756105C324F27C20093F26BEEB2655F803977D5C2EB6B408B8E404AFC201CC15D2");
const QString testToxId =
    QStringLiteral("A369865720756105C324F27C20093F26BEEB2655F803977D5C2EB6B408B8E404AFC201CC15D1");
const QString newNoSpam =
    QStringLiteral("A369865720756105C324F27C20093F26BEEB2655F803977D5C2EB6B408B8E40476ECD68C1BBF");
const QString echoToxId =
    QStringLiteral("76518406F6A9F2217E8DC487CC783C25CC16A15EB36FF32E335A235342C48A39218F515C39A6");

class TestToxId : public QObject
{
    Q_OBJECT
private slots:
    void toStringTest();
    void equalTest();
    void notEqualTest();
    void clearTest();
    void copyTest();
    void validationTest();
};

void TestToxId::toStringTest()
{
    ToxId toxId(testToxId);
    QVERIFY(testToxId == toxId.toString());
}

void TestToxId::equalTest()
{
    ToxId toxId1(testToxId);
    ToxId toxId2(newNoSpam);
    QVERIFY(toxId1 == toxId2);
    QVERIFY(!(toxId1 != toxId2));
}

void TestToxId::notEqualTest()
{
    ToxId toxId1(testToxId);
    ToxId toxId2(echoToxId);
    QVERIFY(toxId1 != toxId2);
}

void TestToxId::clearTest()
{
    ToxId empty;
    ToxId test(testToxId);
    QVERIFY(empty != test);
    test.clear();
    QVERIFY(empty == test);
}

void TestToxId::copyTest()
{
    ToxId src(testToxId);
    ToxId copy = src;
    QVERIFY(copy == src);
}

void TestToxId::validationTest()
{
    QVERIFY(ToxId(testToxId).isValid());
    QVERIFY(ToxId(newNoSpam).isValid());
    QVERIFY(!ToxId(corrupted).isValid());
}

QTEST_GUILESS_MAIN(TestToxId)
#include "toxid_test.moc"
