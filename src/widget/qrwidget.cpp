/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */


#include "qrwidget.h"

#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QPainter>

#include <cerrno>
#include <qrencode.h>

/**
 * @file qrwidget.cpp
 * @link https://stackoverflow.com/questions/21400254/how-to-draw-a-qr-code-with-qt-in-native-c-c
 */

QRWidget::QRWidget(QWidget* parent)
    : QWidget(parent)
    , data("0")
// Note: The encoding fails with empty string so I just default to something else.
// Use the setQRData() call to change this.
{
    // size of the qimage might be problematic in the future, but it works for me
    size.setWidth(480);
    size.setHeight(480);
    image = new QImage(size, QImage::Format_RGB32);
}

QRWidget::~QRWidget()
{
    delete image;
}

void QRWidget::setQRData(const QString& data_)
{
    data = data_;
    paintImage();
}

QImage* QRWidget::getImage()
{
    return image;
}

/**
 * @brief QRWidget::saveImage
 * @param path Full path to the file with extension.
 * @return indicate if saving was successful.
 */
bool QRWidget::saveImage(QString path)
{
    // 0 - image format same as file extension, 75-quality, png file is ~6.3kb
    return image->save(path, nullptr, 75);
}

// http://stackoverflow.com/questions/21400254/how-to-draw-a-qr-code-with-qt-in-native-c-c
void QRWidget::paintImage()
{
    QPainter painter(image);
    // NOTE: I have hardcoded some parameters here that would make more sense as variables.
    // ECLEVEL_M is much faster recognizable by barcodescanner any any other type
    // https://fukuchi.org/works/qrencode/manual/qrencode_8h.html#a4cebc3c670efe1b8866b14c42737fc8f
    // any mode other than QR_MODE_8 or QR_MODE_KANJI results in EINVAL. First 1 is version, second
    // is case sensitivity
    const std::string dataString = data.toStdString();
    QRcode* qr = QRcode_encodeString(dataString.c_str(), 1, QR_ECLEVEL_M, QR_MODE_8, 1);

    if (qr != nullptr) {
        const QColor fg("black");
        const QColor bg("white");
        painter.setBrush(bg);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, size.width(), size.height());
        painter.setBrush(fg);
        painter.scale(0.96, 0.96);
        painter.translate(size.width() * 0.02, size.height() * 0.02);
        const int s = qr->width > 0 ? qr->width : 1;
        const double w = width();
        const double h = height();
        const double aspect = w / h;
        const double scale = ((aspect > 1.0) ? h : w) / s;

        for (int y = 0; y < s; ++y) {
            const int yy = y * s;
            for (int x = 0; x < s; ++x) {
                const int xx = yy + x;
                const unsigned char b = qr->data[xx];
                if ((b & 0x01) != 0) {
                    const double rx1 = x * scale;
                    const double ry1 = y * scale;
                    const QRectF r(rx1, ry1, scale, scale);
                    painter.drawRects(&r, 1);
                }
            }
        }
        QRcode_free(qr);
    } else {
        const QColor error("red");
        painter.setBrush(error);
        painter.drawRect(0, 0, width(), height());
        qDebug() << "QR FAIL:" << strerror(errno);
    }

    qr = nullptr;
}
