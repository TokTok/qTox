/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QLabel>

class QLineEdit;

class CroppingLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CroppingLabel(QWidget* parent = nullptr);

public slots:
    void editBegin();
    void setEditable(bool editable);
    void setElideMode(Qt::TextElideMode elide);

    QString fullText();

public slots:
    void setText(const QString& text);
    void setPlaceholderText(const QString& text);
    void minimizeMaximumWidth();

signals:
    void editFinished(const QString& newText);
    void editRemoved();
    void clicked();

protected:
    void paintEvent(QPaintEvent* paintEvent) override;
    void setElidedText();
    void hideTextEdit();
    void showTextEdit();
    void resizeEvent(QResizeEvent* ev) final;
    QSize sizeHint() const final;
    QSize minimumSizeHint() const final;
    void mouseReleaseEvent(QMouseEvent* e) final;

private slots:
    void editingFinished();

private:
    QString origText;
    QLineEdit* textEdit;
    bool blockPaintEvents;
    bool editable;
    Qt::TextElideMode elideMode;
};
