/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */


#pragma once

#include <QBrush>
#include <QPolygon>
#include <QRect>
#include <QWidget>

class QPaintEvent;
class QShowEvent;
class Settings;
class Style;

class CallConfirmWidget final : public QWidget
{
    Q_OBJECT
public:
    CallConfirmWidget(Settings& settings, Style& style, const QWidget* anchor_);

signals:
    void accepted();
    void rejected();

public slots:
    void reposition();

protected:
    void paintEvent(QPaintEvent* event) final;
    void showEvent(QShowEvent* event) final;
    void hideEvent(QHideEvent* event) final;
    bool eventFilter(QObject* object, QEvent* event) final;

private:
    const QWidget* anchor;

    QRect mainRect;
    QPolygon spikePoly;
    QBrush brush;

    const int rectW, rectH;
    const int spikeW, spikeH;
    const int roundedFactor;
    const qreal rectRatio;
};
