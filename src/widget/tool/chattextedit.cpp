/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "chattextedit.h"

#include "src/widget/translator.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMimeData>

ChatTextEdit::ChatTextEdit(QWidget* parent)
    : QTextEdit(parent)
{
    retranslateUi();
    setAcceptRichText(false);
    setAcceptDrops(false);

    Translator::registerHandler(std::bind(&ChatTextEdit::retranslateUi, this), this);
}

ChatTextEdit::~ChatTextEdit()
{
    Translator::unregister(this);
}

void ChatTextEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();
    if ((key == Qt::Key_Enter || key == Qt::Key_Return) && !(event->modifiers() & Qt::ShiftModifier)) {
        emit enterPressed();
        return;
    }
    if (key == Qt::Key_Escape) {
        emit escapePressed();
        return;
    }
    if (key == Qt::Key_Tab) {
        if (event->modifiers())
            event->ignore();
        else {
            emit tabPressed();
            event->ignore();
        }
        return;
    }
    if (key == Qt::Key_Up && toPlainText().isEmpty()) {
        setPlainText(lastMessage);
        setFocus();
        moveCursor(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::MoveAnchor);
        return;
    }
    if (event->matches(QKeySequence::Paste) && pasteIfImage(event)) {
        return;
    }
    emit keyPressed();
    QTextEdit::keyPressEvent(event);
}

void ChatTextEdit::setLastMessage(QString lm)
{
    lastMessage = lm;
}

void ChatTextEdit::retranslateUi()
{
    setPlaceholderText(tr("Type your message here..."));
}

void ChatTextEdit::sendKeyEvent(QKeyEvent* event)
{
    emit keyPressEvent(event);
}

bool ChatTextEdit::pasteIfImage(QKeyEvent* event)
{
    std::ignore = event;
    const QClipboard* const clipboard = QApplication::clipboard();
    if (!clipboard) {
        return false;
    }

    const QMimeData* const mimeData = clipboard->mimeData();
    if (!mimeData || !mimeData->hasImage()) {
        return false;
    }

    const QPixmap pixmap(clipboard->pixmap());
    if (pixmap.isNull()) {
        return false;
    }

    emit pasteImage(pixmap);
    return true;
}
