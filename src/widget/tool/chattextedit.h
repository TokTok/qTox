/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QTextEdit>

class ChatTextEdit final : public QTextEdit
{
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget* parent = nullptr);
    ~ChatTextEdit();
    void setLastMessage(QString lm);
    void sendKeyEvent(QKeyEvent* event);

signals:
    void enterPressed();
    void escapePressed();
    void tabPressed();
    void keyPressed();
    void pasteImage(const QPixmap& pixmap);

protected:
    void keyPressEvent(QKeyEvent* event) final;

private:
    void retranslateUi();
    bool pasteIfImage(QKeyEvent* event);

private:
    QString lastMessage;
};
