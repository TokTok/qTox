/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/chatlog/content/spinner.h"

#include <QImage>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryDir>
#include <QWidget>
#include <QtTest/QtTest>

#include <cstring>
#include <new>

class TestSpinner : public QObject
{
    Q_OBJECT
private slots:
    void testFirstPaint();
};

// Fill the memory with NaN floats before building the spinner in it, so we
// catch the first paint reading an angle nothing has set yet.
void TestSpinner::testFirstPaint()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString imagePath = dir.filePath("dot.png");
    QImage dot(16, 16, QImage::Format_ARGB32);
    dot.fill(Qt::red);
    QVERIFY(dot.save(imagePath));

    alignas(Spinner) unsigned char storage[sizeof(Spinner)];
    std::memset(storage, 0xFF, sizeof(storage));
    auto* spinner = new (storage) Spinner(imagePath, QSize(16, 16), 360.0 / 1.6);

    // Let the fade-in finish first, or the pixel is still transparent.
    QTest::qWait(500);

    QImage image(32, 32, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.translate(16, 16);
    QStyleOptionGraphicsItem option;
    spinner->paint(&painter, &option, nullptr);
    painter.end();

    QCOMPARE(image.pixelColor(16, 16), QColor(Qt::red));

    spinner->~Spinner();
}

QTEST_MAIN(TestSpinner)
#include "spinner_test.moc"
