/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QLabel>

class MaskablePixmapWidget final : public QLabel
{
    Q_OBJECT
public:
    MaskablePixmapWidget(QWidget* parent, QSize size, QString maskName_ = QString());
    ~MaskablePixmapWidget() override;
    void autopickBackground();
    void setClickable(bool clickable_);
    void setPixmap(const QPixmap& pmap);
    QPixmap getPixmap() const;
    void setSize(QSize size);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent* event) final;

private:
    void updatePixmap();

private:
    QPixmap pixmap, mask, unscaled;
    QPixmap* renderTarget;
    QString maskName;
    bool clickable;
};
