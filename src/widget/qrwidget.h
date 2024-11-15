/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */


#pragma once

#include <QWidget>

class QRWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QRWidget(QWidget* parent = nullptr);
    ~QRWidget();
    void setQRData(const QString& data_);
    QImage* getImage();
    bool saveImage(QString path);

private:
    QString data;
    void paintImage();
    QImage* image;
    QSize size;
};
