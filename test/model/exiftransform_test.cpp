/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2020 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/model/exiftransform.h"

#include <QPainter>
#include <QTest>

namespace {
const auto rowColor = QColor(Qt::green).rgb();
const auto colColor = QColor(Qt::blue).rgb();

enum class Side
{
    top,
    bottom,
    left,
    right
};

QPoint getPosition(Side side)
{
    int x, y;
    switch (side) {
    case Side::top: {
        x = 1;
        y = 0;
        break;
    }
    case Side::bottom: {
        x = 1;
        y = 2;
        break;
    }
    case Side::left: {
        x = 0;
        y = 1;
        break;
    }
    case Side::right: {
        x = 2;
        y = 1;
        break;
    }
    }

    return {x, y};
}

QRgb getColor(const QImage& image, Side side)
{
    return image.pixel(getPosition(side));
}
} // namespace

class TestExifTransform : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void testTopLeft();
    void testTopRight();
    void testBottomRight();
    void testBottomLeft();
    void testLeftTop();
    void testRightTop();
    void testRightBottom();
    void testLeftBottom();
    void testInputTooShort();

private:
    QImage inputImage;
};

void TestExifTransform::init()
{
    inputImage = QImage(QSize(3, 3), QImage::Format_RGB32);
    QPainter painter(&inputImage);
    painter.fillRect(QRect(0, 0, 3, 3), Qt::white);
    // First row has a green dot in the middle
    painter.setPen(rowColor);
    painter.drawPoint(getPosition(Side::top));
    // First column has a blue dot in the middle
    painter.setPen(colColor);
    painter.drawPoint(getPosition(Side::left));
}

void TestExifTransform::testTopLeft()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::TopLeft);
    QVERIFY(getColor(image, Side::top) == rowColor);
    QVERIFY(getColor(image, Side::left) == colColor);
}

void TestExifTransform::testTopRight()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::TopRight);
    QVERIFY(getColor(image, Side::top) == rowColor);
    QVERIFY(getColor(image, Side::right) == colColor);
}

void TestExifTransform::testBottomRight()
{
    auto image =
        ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::BottomRight);
    QVERIFY(getColor(image, Side::bottom) == rowColor);
    QVERIFY(getColor(image, Side::right) == colColor);
}

void TestExifTransform::testBottomLeft()
{
    auto image =
        ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::BottomLeft);
    QVERIFY(getColor(image, Side::bottom) == rowColor);
    QVERIFY(getColor(image, Side::left) == colColor);
}

void TestExifTransform::testLeftTop()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::LeftTop);
    QVERIFY(getColor(image, Side::left) == rowColor);
    QVERIFY(getColor(image, Side::top) == colColor);
}

void TestExifTransform::testRightTop()
{
    auto image = ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::RightTop);
    QVERIFY(getColor(image, Side::right) == rowColor);
    QVERIFY(getColor(image, Side::top) == colColor);
}

void TestExifTransform::testRightBottom()
{
    auto image =
        ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::RightBottom);
    QVERIFY(getColor(image, Side::right) == rowColor);
    QVERIFY(getColor(image, Side::bottom) == colColor);
}

void TestExifTransform::testLeftBottom()
{
    auto image =
        ExifTransform::applyTransformation(inputImage, ExifTransform::Orientation::LeftBottom);
    QVERIFY(getColor(image, Side::left) == rowColor);
    QVERIFY(getColor(image, Side::bottom) == colColor);
}

void TestExifTransform::testInputTooShort()
{
    const QByteArray garbage = QByteArray::fromBase64(
        "AEV4aWYAAE1NACoAAAAA0v///7p6h2kAaWYAAAAAAABNeGlmAABRMAAjGEQAAAAAAAAAQAAAAAAA"
        "/////////////////wEBASMBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBARIAAQAAAAEB"
        "AAABAAEBAQEjAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEAAAEBAADCwsLCwsLCwsLCwsLCwsLCwsLC"
        "wsLCAQEBAQEBAQEBAQEBAQAAAQEAAAABAAEBAQEjAQEBAAAAAAAAAAD/////////////////BgD/"
        "/////wAAAQABAQEBIwEBAQEjAQEB/////wEBAQEBAQEBAQEBAQEBAQEB");

    ExifTransform::getOrientation(garbage.mid(1));
}

QTEST_APPLESS_MAIN(TestExifTransform)
#include "exiftransform_test.moc"
