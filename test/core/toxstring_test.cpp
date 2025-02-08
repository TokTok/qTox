/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2018-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/core/toxstring.h"

#include "src/widget/form/settings/generalform.h" // getLocales

#include <QByteArray>
#include <QString>
#include <QtTest/QtTest>

class TestToxString : public QObject
{
    Q_OBJECT
private slots:
    void QStringTest();
    void QByteArrayTest();
    void uint8_tTest();
    void emptyQStrTest();
    void emptyQByteTest();
    void emptyUINT8Test();
    void nullptrUINT8Test();
    void localesTest();
    void filterTest();
    void emojiTest();
    void handshakeEmojiTest();
    void coloredEmojiTest();
    void combiningCharacterTest();
    void multiLineTest();
    void tabTest();
    void trifaFormatTest();

private:
    /* Test Strings */
    //"My Test String" - test text
    static const QString testStr;
    static const QByteArray testByte;
    static const uint8_t* testUINT8;
    static const int lengthUINT8;

    //"" - empty test text
    static const QString emptyStr;
    static const QByteArray emptyByte;
    static const uint8_t* emptyUINT8;
    static const int emptyLength;
};


/* Test Strings */
//"My Test String" - test text
const QString TestToxString::testStr = QStringLiteral("My Test String");
const QByteArray TestToxString::testByte = QByteArrayLiteral("My Test String");
const uint8_t* TestToxString::testUINT8 = reinterpret_cast<const uint8_t*>("My Test String");
const int TestToxString::lengthUINT8 = 14;

//"" - empty test text
const QString TestToxString::emptyStr = QStringLiteral("");
const QByteArray TestToxString::emptyByte = QByteArrayLiteral("");
const uint8_t* TestToxString::emptyUINT8 = reinterpret_cast<const uint8_t*>("");
const int TestToxString::emptyLength = 0;

/**
 * @brief Use QString as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::QStringTest()
{
    // Create Test Object with QString constructor
    ToxString test(testStr);

    // Check QString
    QString test_string = test.getQString();
    QVERIFY(testStr == test_string);

    // Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(testByte == test_byte);

    // Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(lengthUINT8 == test_size);
    for (int i = 0; i <= lengthUINT8; i++) {
        QVERIFY(testUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use QByteArray as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::QByteArrayTest()
{
    // Create Test Object with QByteArray constructor
    ToxString test(testByte);

    // Check QString
    QString test_string = test.getQString();
    QVERIFY(testStr == test_string);

    // Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(testByte == test_byte);

    // Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(lengthUINT8 == test_size);
    for (int i = 0; i <= lengthUINT8; i++) {
        QVERIFY(testUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use uint8t* and size_t as input data, check output: QString, QByteArray, size_t and
 * uint8_t QVERIFY(expected == result);
 */
void TestToxString::uint8_tTest()
{
    // Create Test Object with uint8_t constructor
    ToxString test(testUINT8, lengthUINT8);

    // Check QString
    QString test_string = test.getQString();
    QVERIFY(testStr == test_string);

    // Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(testByte == test_byte);

    // Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(lengthUINT8 == test_size);
    for (int i = 0; i <= lengthUINT8; i++) {
        QVERIFY(testUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use empty QString as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::emptyQStrTest()
{
    // Create Test Object with QString constructor
    ToxString test(emptyStr);

    // Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    // Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    // Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++) {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use empty QByteArray as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::emptyQByteTest()
{
    // Create Test Object with QByteArray constructor
    ToxString test(emptyByte);

    // Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    // Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    // Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++) {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use empty uint8_t as input data, check output: QString, QByteArray, size_t and uint8_t
 *        QVERIFY(expected == result);
 */
void TestToxString::emptyUINT8Test()
{
    // Create Test Object with uint8_t constructor
    ToxString test(emptyUINT8, emptyLength);

    // Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    // Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    // Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++) {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Use nullptr and size_t 5 as input data, check output: QString, QByteArray, size_t and
 * uint8_t QVERIFY(expected == result);
 */
void TestToxString::nullptrUINT8Test()
{
    // Create Test Object with uint8_t constructor
    ToxString test(nullptr, 5); // nullptr and length = 5

    // Check QString
    QString test_string = test.getQString();
    QVERIFY(emptyStr == test_string);

    // Check QByteArray
    QByteArray test_byte = test.getBytes();
    QVERIFY(emptyByte == test_byte);

    // Check uint8_t pointer
    const uint8_t* test_int = test.data();
    size_t test_size = test.size();
    QVERIFY(emptyLength == test_size);
    for (int i = 0; i <= emptyLength; i++) {
        QVERIFY(emptyUINT8[i] == test_int[i]);
    }
}

/**
 * @brief Check that we can encode and decode all native locale names.
 *
 * This is a smoke test as opposed to anything comprehensive, but it ensures that scripts used
 * in the languages we have translations for can be encoded and decoded.
 */
void TestToxString::localesTest()
{
    const QStringList& locales = GeneralForm::getLocales();
    for (const QString& locale : locales) {
        const QString lang = QLocale(locale).nativeLanguageName();
        ToxString test(lang);
        const QString test_string = test.getQString();
        QCOMPARE(lang, test_string);
    }
}

/**
 * @brief Check that we filter out non-printable characters.
 */
void TestToxString::filterTest()
{
    const struct TestCase
    {
        QString input;
        QString expected;
    } testCases[] = {
        {QStringLiteral("Hello, World!"), QStringLiteral("Hello, World!")},
        {QStringLiteral("Hello, \x00World!"), QStringLiteral("Hello, World!")},
        {QStringLiteral("Hello, \x01World!"), QStringLiteral("Hello, World!")},
        {QStringLiteral("Hello, \x7FWorld!"), QStringLiteral("Hello, World!")},
        {QStringLiteral("Hello, \x80World!"), QStringLiteral("Hello, World!")},
    };
    for (const auto& testCase : testCases) {
        QCOMPARE(ToxString(testCase.input).getQString(), testCase.expected);
    }
}

/**
 * @brief Check that we can encode and decode emojis.
 *
 * These are high code point characters that are not single code units in UTF-16.
 */
void TestToxString::emojiTest()
{
    const QString testCases[] = {
        QStringLiteral("🙂"), QStringLiteral("🙁"), QStringLiteral("🤣"), QStringLiteral("🤷"),
        QStringLiteral("🤼"), QStringLiteral("🥃"), QStringLiteral("🥌"), QStringLiteral("🥐"),
    };

    for (const auto& testCase : testCases) {
        QCOMPARE(ToxString(testCase).getQString(), testCase);
    }
}

/**
 * @brief Check that we can encode and decode emojis with color modifiers.
 *
 * These use modifier characters to change the color of the emoji.
 */
void TestToxString::coloredEmojiTest()
{
    const QString testCases[] = {
        QStringLiteral("👨🏻‍👩🏻‍👧🏻‍👦🏻"),
        QStringLiteral("👨🏼‍👩🏼‍👧🏼‍👦🏼"),
        QStringLiteral("👨🏽‍👩🏽‍👧🏽‍👦🏽"),
        QStringLiteral("👨🏾‍👩🏾‍👧🏾‍👦🏾"),
        QStringLiteral("👨🏿‍👩🏿‍👧🏿‍👦🏿"),
    };
    for (const auto& testCase : testCases) {
        QCOMPARE(ToxString(testCase).getQString(), testCase);
    }
}

void TestToxString::handshakeEmojiTest()
{
    if (QChar::category(0x1FAF1) == QChar::Other_NotAssigned) {
        QSKIP("Emoji U+1FAF1 (Rightwards Hand) not supported by Qt");
    }
    const QString testCases[] = {
        QStringLiteral("🫱🏼‍🫲🏽"),
    };

    for (const auto& testCase : testCases) {
        QCOMPARE(ToxString(testCase).getQString(), testCase);
    }
}

/**
 * @brief Check that we can encode and decode combining characters.
 *
 * These are codepoints that add diacritics and other decorations around characters.
 */
void TestToxString::combiningCharacterTest()
{
    const struct TestCase
    {
        QString input;
        QString expected;
    } testCases[] = {
        // U+0303 Combining Tilde
        // U+0320 Combining Minus Sign Below
        // U+0337 Combining Short Solidus Overlay
        {QStringLiteral(u"o\u0303\u0337\u0320"), QStringLiteral(u"o\u0303\u0337\u0320")},
    };

    for (const auto& testCase : testCases) {
        QCOMPARE(ToxString(testCase.input).getQString(), testCase.expected);
    }
}

/**
 * @brief Check that we trim TRIfA suffix only if it has a right shape and user set this option.
 */
void TestToxString::trifaFormatTest()
{
    const struct TestCase
    {
        const uint8_t* text;
        const size_t length;
        const bool fix_trifa;
        const QString expected;
    } testCases[] = {
        {reinterpret_cast<const uint8_t*>("Hello\0\0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx4444"), 43,
         true, QStringLiteral("Hello[TRIfA_SUFFIX]xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx1997-10-02T23:54:28Z[/TRIfA_SUFFIX]")},
        {reinterpret_cast<const uint8_t*>("Hello\0\0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx4444"), 43,
         false, QStringLiteral("Helloxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx4444")},
        {reinterpret_cast<const uint8_t*>("Hello\0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx4444"), 42, true,
         QStringLiteral("Helloxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx4444")},
        {reinterpret_cast<const uint8_t*>("Hello\0\0xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx4444"), 42, true,
         QStringLiteral("Helloxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx4444")},
    };
    for (const auto& testCase : testCases) {
        QCOMPARE(ToxString(testCase.text, testCase.length, testCase.fix_trifa).getQString(),
                 testCase.expected);
    }
}

/**
 * @brief Check that we can encode and decode multi-line strings.
 */
void TestToxString::multiLineTest()
{
    const struct TestCase
    {
        QString input;
        QString expected;
    } testCases[] = {
        {QStringLiteral("Hello,\nWorld!"), QStringLiteral("Hello,\nWorld!")},
        {QStringLiteral("Hello,\r\nWorld!"), QStringLiteral("Hello,\r\nWorld!")},
        {QStringLiteral("Hello,\rWorld!"), QStringLiteral("Hello,\rWorld!")},
    };

    for (const auto& testCase : testCases) {
        QCOMPARE(ToxString(testCase.input).getQString(), testCase.expected);
    }
}

/**
 * @brief Check that we can encode and decode tabs.
 */
void TestToxString::tabTest()
{
    QCOMPARE(ToxString(QStringLiteral("Hello,\tWorld!")).getQString(),
             QStringLiteral("Hello,\tWorld!"));
}

QTEST_GUILESS_MAIN(TestToxString)
#include "toxstring_test.moc"
