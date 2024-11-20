/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2021 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/persistence/ismileysettings.h"
#include "src/persistence/smileypack.h"
#include "util/interface.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QString>
#include <QtTest/QtTest>

#include <QGuiApplication>

#include <memory>

class MockSettings : public QObject, public ISmileySettings
{
    Q_OBJECT
public:
    QString getSmileyPack() const override;
    SIGNAL_IMPL(MockSettings, smileyPackChanged, const QString& name)
};

QString MockSettings::getSmileyPack() const
{
    return ":/smileys/emojione/emoticons.xml";
}

class TestSmileyPack : public QObject
{
    Q_OBJECT
public:
    TestSmileyPack();

private slots:
    void testSmilifySingleCharEmoji();
    void testSmilifyMultiCharEmoji();
    void testSmilifyAsciiEmoticon();

private:
    std::unique_ptr<QGuiApplication> app;
    std::unique_ptr<MockSettings> settings;
};

TestSmileyPack::TestSmileyPack()
{
    static char arg1[]{"QToxSmileyPackTestApp"};
    static char arg2[]{"-platform"};
    static char arg3[]{"offscreen"};
    static char* qtTestAppArgv[] = {arg1, arg2, arg3};
    static int qtTestAppArgc = 3;

    app = std::make_unique<QGuiApplication>(qtTestAppArgc, qtTestAppArgv);
    settings = std::make_unique<MockSettings>();
}

/**
 * @brief Test that single-character emojis (non-ascii) are correctly smilified
 */
void TestSmileyPack::testSmilifySingleCharEmoji()
{
    SmileyPack smileyPack{*settings};

    auto result = smileyPack.smileyfied("😊");
    QVERIFY(result == SmileyPack::getAsRichText("😊"));

    result = smileyPack.smileyfied("Some😊Letters");
    QVERIFY(result == "Some" + SmileyPack::getAsRichText("😊") + "Letters");
}

/**
 * @brief Test that multi-character emojis (non-ascii) are correctly smilified
 *  and not incorrectly matched against single-char counterparts
 */
void TestSmileyPack::testSmilifyMultiCharEmoji()
{
    SmileyPack smileyPack{*settings};

    auto result = smileyPack.smileyfied("🇬🇧");
    QVERIFY(result == SmileyPack::getAsRichText("🇬🇧"));

    result = smileyPack.smileyfied("Some🇬🇧Letters");
    QVERIFY(result == "Some" + SmileyPack::getAsRichText("🇬🇧") + "Letters");

    // This verifies that multi-char emojis are not accidentally
    // considered a multichar ascii smiley
    result = smileyPack.smileyfied("🇫🇷🇬🇧");
    QVERIFY(result == SmileyPack::getAsRichText("🇫🇷") + SmileyPack::getAsRichText("🇬🇧"));
}


/**
 * @brief Test that single character emojis (non-ascii) are correctly smilified
 *  and not when surrounded by non-punctuation and non-whitespace chars
 */
void TestSmileyPack::testSmilifyAsciiEmoticon()
{
    SmileyPack smileyPack{*settings};

    auto result = smileyPack.smileyfied(":-)");
    QVERIFY(result == SmileyPack::getAsRichText(":-)"));

    const auto testMsg = QStringLiteral("Some:-)Letters");
    result = smileyPack.smileyfied(testMsg);

    // Nothing has changed. Ascii smileys are only considered
    // when they are surrounded by white space
    QVERIFY(result == testMsg);

    result = smileyPack.smileyfied("  :-)  ");
    QVERIFY(result == "  " + SmileyPack::getAsRichText(":-)") + "  ");
}


QTEST_GUILESS_MAIN(TestSmileyPack)
#include "smileypack_test.moc"
