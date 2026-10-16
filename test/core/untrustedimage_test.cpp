/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/core/untrustedimage.h"

#include <QBuffer>
#include <QByteArray>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QtTest/QtTest>

namespace {
QByteArray encode(const char* format, Qt::GlobalColor color = Qt::red)
{
    QImage image{4, 4, QImage::Format_RGB32};
    image.fill(color);
    QByteArray data;
    QBuffer buffer{&data};
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format);
    return data;
}

QByteArray svgDocument(const QByteArray& body)
{
    return "<svg xmlns=\"http://www.w3.org/2000/svg\" "
           "xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"4\" height=\"4\">"
           + body + "</svg>";
}

QByteArray svgImage(const QByteArray& href)
{
    return svgDocument("<image xlink:href=\"" + href + R"(" width="4" height="4"/>)");
}

QByteArray dataUrl(const QByteArray& mime, const QByteArray& bytes)
{
    return "data:" + mime + ";base64," + bytes.toBase64();
}

bool haveSvg()
{
    return QImageReader::supportedImageFormats().contains("svg");
}

const QByteArray xpm = QByteArrayLiteral("/* XPM */\n"
                                         "static const char *x[] = {\n"
                                         "\"2 2 1 1\",\n"
                                         "\"a c #FF0000\",\n"
                                         "\"aa\",\n"
                                         "\"aa\"};\n");

const QByteArray redRect = QByteArrayLiteral("<rect width=\"4\" height=\"4\" fill=\"red\"/>");
} // namespace

class TestUntrustedImage : public QObject
{
    Q_OBJECT
private slots:
    void decodesRasterFormats();
    void rejectsEmpty();
    void rejectsOtherFormats();
    void rejectsPngSignatureWithOtherFormat();
    void rejectsTruncatedPng();
    void decodesSelfContainedSvg();
    void decodesSvgWithEmbeddedImage();
    void rejectsExternalReferences_data();
    void rejectsExternalReferences();
};

void TestUntrustedImage::decodesRasterFormats()
{
    const QImage png = UntrustedImage::decode(encode("PNG"));
    QCOMPARE(png.size(), QSize(4, 4));
    QCOMPARE(png.pixelColor(0, 0), QColor(Qt::red));

    for (const char* format : {"jpeg", "webp", "qoi"}) {
        if (!QImageWriter::supportedImageFormats().contains(format)) {
            continue;
        }
        QCOMPARE(UntrustedImage::decode(encode(format)).size(), QSize(4, 4));
    }

    if (QImageReader::supportedImageFormats().contains("gif")) {
        // 1x1 GIF89a.
        const QByteArray gif =
            QByteArray::fromBase64("R0lGODlhAQABAIAAAP8AAAAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==");
        QCOMPARE(UntrustedImage::decode(gif).size(), QSize(1, 1));
    }
}

void TestUntrustedImage::rejectsEmpty()
{
    QVERIFY(UntrustedImage::decode({}).isNull());
}

void TestUntrustedImage::rejectsOtherFormats()
{
    const QByteArray bmp = encode("BMP");
    // Format auto-detection accepts these, so they'd reach Qt's decoders.
    QVERIFY(!QImage::fromData(bmp).isNull());
    QVERIFY(!QImage::fromData(xpm).isNull());

    QVERIFY(UntrustedImage::decode(bmp).isNull());
    QVERIFY(UntrustedImage::decode(xpm).isNull());
}

void TestUntrustedImage::rejectsPngSignatureWithOtherFormat()
{
    const QByteArray signature = encode("PNG").left(8);
    QVERIFY(UntrustedImage::decode(signature + encode("BMP")).isNull());
    QVERIFY(UntrustedImage::decode(signature + xpm).isNull());
    QVERIFY(UntrustedImage::decode(signature + svgDocument(redRect)).isNull());
}

void TestUntrustedImage::rejectsTruncatedPng()
{
    const QByteArray png = encode("PNG");
    QVERIFY(UntrustedImage::decode(png.left(png.size() / 2)).isNull());
}

void TestUntrustedImage::decodesSelfContainedSvg()
{
    if (!haveSvg()) {
        QSKIP("Qt SVG image plugin not available");
    }

    QImage image = UntrustedImage::decode(svgDocument(redRect));
    QCOMPARE(image.size(), QSize(4, 4));
    QCOMPARE(image.pixelColor(0, 0), QColor(Qt::red));

    // Same-document references are fine.
    const QByteArray gradient =
        "<defs><linearGradient id=\"g\"><stop offset=\"0\" stop-color=\"red\"/>"
        "<stop offset=\"1\" stop-color=\"red\"/></linearGradient>"
        "<rect id=\"r\" width=\"4\" height=\"4\" fill=\"url(#g)\"/></defs>";
    image = UntrustedImage::decode(svgDocument(gradient + "<use xlink:href=\"#r\"/>"));
    QCOMPARE(image.pixelColor(1, 1), QColor(Qt::red));
    QVERIFY(!UntrustedImage::decode(
                 svgDocument(gradient
                             + "<rect width=\"4\" height=\"4\" style=\"fill: url('#g')\"/>"
                               "<style>rect { stroke: url(#g); }</style>"))
                 .isNull());
}

void TestUntrustedImage::decodesSvgWithEmbeddedImage()
{
    if (!haveSvg()) {
        QSKIP("Qt SVG image plugin not available");
    }

    const QByteArray url = dataUrl("image/png", encode("PNG", Qt::blue));
    QImage image = UntrustedImage::decode(svgImage(url));
    QCOMPARE(image.pixelColor(1, 1), QColor(Qt::blue));

    // Base64 wrapped over several lines (some editors write it).
    QByteArray wrapped = url;
    for (qsizetype i = url.indexOf(',') + 20; i < wrapped.size(); i += 21) {
        wrapped.insert(i, '\n');
    }
    image = UntrustedImage::decode(svgImage(wrapped));
    QCOMPARE(image.pixelColor(1, 1), QColor(Qt::blue));
}

void TestUntrustedImage::rejectsExternalReferences_data()
{
    QTest::addColumn<QByteArray>("data");

    const QByteArray png = encode("PNG", Qt::blue);
    const QByteArray nestedSvg = svgImage("/tmp/external.png");

    QTest::newRow("absolute path") << svgImage("/tmp/external.png");
    QTest::newRow("relative path") << svgImage("external.png");
    QTest::newRow("windows share path") << svgImage(R"(\\server\share\image.png)");
    QTest::newRow("file url") << svgImage("file:///tmp/external.png");
    QTest::newRow("http url") << svgImage("http://127.0.0.1:9/external.png");
    QTest::newRow("svg2 href") << svgDocument(
        R"(<image href="/tmp/external.png" width="4" height="4"/>)");
    QTest::newRow("other xlink prefix")
        << QByteArray("<svg xmlns=\"http://www.w3.org/2000/svg\" "
                      "xmlns:x=\"http://www.w3.org/1999/xlink\" width=\"4\" height=\"4\">"
                      "<image x:href=\"/tmp/external.png\" width=\"4\" height=\"4\"/></svg>");
    QTest::newRow("use external") << svgDocument("<use xlink:href=\"other.svg#r\"/>");
    QTest::newRow("data svg") << svgImage(dataUrl("image/svg+xml", nestedSvg));
    QTest::newRow("data svg labelled png") << svgImage(dataUrl("image/png", nestedSvg));
    QTest::newRow("data bmp") << svgImage(dataUrl("image/png", encode("BMP")));
    QTest::newRow("data xpm") << svgImage(dataUrl("image/png", xpm));
    QTest::newRow("data not base64") << svgImage("data:image/png," + png.toPercentEncoding());
    QTest::newRow("data uppercase scheme") << svgImage("DATA:image/png;base64," + png.toBase64());
    QTest::newRow("xml-stylesheet")
        << R"(<?xml-stylesheet type="text/css" href="/tmp/external.css"?>)" + svgDocument(redRect);
    QTest::newRow("doctype entity")
        << "<!DOCTYPE svg [<!ENTITY e SYSTEM \"/tmp/external\">]>" + svgDocument("&e;");
    QTest::newRow("css import") << svgDocument("<style>@import url(/tmp/external.css);</style>"
                                               + redRect);
    QTest::newRow("css url") << svgDocument("<style>rect { fill: url(/tmp/x.svg#g); }</style>"
                                            + redRect);
    QTest::newRow("css cdata url")
        << svgDocument("<style><![CDATA[rect { fill: url(/tmp/x.svg#g); }]]></style>" + redRect);
    QTest::newRow("style attribute url")
        << svgDocument("<rect width=\"4\" height=\"4\" style=\"fill: url(/tmp/x.svg#g)\"/>");
    QTest::newRow("css escape") << svgDocument(
        "<rect width=\"4\" height=\"4\" style=\"fill: u\\72l(/tmp/x.svg#g)\"/>");
    QTest::newRow("presentation attribute url")
        << svgDocument("<rect width=\"4\" height=\"4\" fill=\"url(/tmp/x.svg#g)\"/>");
    QTest::newRow("not svg") << QByteArray("<html><img src=\"/tmp/external.png\"/></html>");
    QTest::newRow("malformed") << QByteArray("<svg xmlns=\"http://www.w3.org/2000/svg\">");
    QTest::newRow("gzip") << QByteArray::fromHex("1f8b0800000000000003");
}

void TestUntrustedImage::rejectsExternalReferences()
{
    QFETCH(QByteArray, data);
    QVERIFY(UntrustedImage::decode(data).isNull());
}

QTEST_GUILESS_MAIN(TestUntrustedImage)
#include "untrustedimage_test.moc"
