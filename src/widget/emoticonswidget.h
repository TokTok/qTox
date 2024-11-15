/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QMenu>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QVector>

#include <memory>

class QIcon;
class SmileyPack;
class Settings;
class Style;

class EmoticonsWidget : public QMenu
{
    Q_OBJECT
public:
    EmoticonsWidget(SmileyPack& smileyPack, Settings& settings, Style& style,
        QWidget* parent = nullptr);

signals:
    void insertEmoticon(QString str);

private slots:
    void onSmileyClicked();
    void onPageButtonClicked();
    void PageButtonsUpdate();

protected:
    void mouseReleaseEvent(QMouseEvent* ev) final;
    void mousePressEvent(QMouseEvent* ev) final;
    void wheelEvent(QWheelEvent* event) final;
    void keyPressEvent(QKeyEvent* e) final;

private:
    QStackedWidget stack;
    QVBoxLayout layout;
    QList<std::shared_ptr<QIcon>> emoticonsIcons;

public:
    QSize sizeHint() const override;
};
