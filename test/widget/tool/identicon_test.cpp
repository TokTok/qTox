/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#include "src/widget/tool/identicon.h"

#include <QColor>
#include <QImage>
#include <QList>
#include <QTest>

#include <QtTest/qtestcase.h>

class TestIdenticon : public QObject
{
    Q_OBJECT

private slots:
    void testIdenticon();
    void testTwoKeysHaveDifferentColors();
    void testParseMatrix();
    void testParseMatrixRejectsWrongSizeStrings();
};

void TestIdenticon::testIdenticon()
{
    const Identicon identicon(QByteArray( //
        "\x7A\x11\x41\x77\xE3\x93\x45\x88"
        "\xEE\xD8\x7E\x6F\xE8\xB1\x8A\xF7"
        "\xDC\x58\x1C\x1F\xBB\x67\xF9\x73"
        "\xEE\x5B\xF0\x7B\x6E\xFA\xBB\x72"));

    const auto matrix = identicon.toMatrix();
    const auto expected = Identicon::Matrix::parse( //
        matrix.colors,                              //
        "00000"
        "11011"
        "00000"
        "01110"
        "10001");
    QCOMPARE(matrix, expected);

    const QList<QString> colors{
        matrix.colors[0].name(),
        matrix.colors[1].name(),
    };
    const QList<QString> expectedColors{
        "#732650",
        "#e6b2b9",
    };
    QCOMPARE(colors, expectedColors);
}

void TestIdenticon::testTwoKeysHaveDifferentColors()
{
    const Identicon identicon1(QByteArray("hello"));
    const Identicon identicon2(QByteArray("hallo"));

    const auto matrix1 = identicon1.toMatrix();
    const auto matrix2 = identicon2.toMatrix();
    QVERIFY(matrix1.colors != matrix2.colors);
}

void TestIdenticon::testParseMatrix()
{
    const auto parsed = Identicon::Matrix::parse( //
        {{QColor(), QColor()}},                   //
        "10001"
        "10101"
        "00100"
        "01110"
        "11011");

    constexpr auto expected = Identicon::Matrix{
        {{QColor(), QColor()}},
        {{
            {{1, 0, 0, 0, 1}},
            {{1, 0, 1, 0, 1}},
            {{0, 0, 1, 0, 0}},
            {{0, 1, 1, 1, 0}},
            {{1, 1, 0, 1, 1}},
        }},
    };
    QCOMPARE(parsed, expected);
}

void TestIdenticon::testParseMatrixRejectsWrongSizeStrings()
{
    QCOMPARE(Identicon::Matrix::parse({}, "101010"), Identicon::Matrix{});
    QCOMPARE(Identicon::Matrix::parse({}, "10101010101010101010101010"), Identicon::Matrix{});
    QCOMPARE(Identicon::Matrix::parse( //
                 {},                   //
                 "10001"
                 "10101"
                 "00100"
                 "01110"
                 "11012"), // 2 is out of range
             Identicon::Matrix{});
}

QTEST_GUILESS_MAIN(TestIdenticon)
#include "identicon_test.moc"
