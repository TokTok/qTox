/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2005-2014 by the Quassel Project <devel@quassel-irc.org>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "tabcompleter.h"

#include <QKeyEvent>
#include <QRegularExpression>

#include "src/model/group.h"
#include "src/widget/tool/chattextedit.h"

/**
 * @file tabcompleter.h
 * @file tabcompleter.cpp
 * These files were taken from the Quassel IRC client source (src/uisupport), and
 * was greatly simplified for use in qTox.
 */

const QString TabCompleter::nickSuffix = QString(": ");

TabCompleter::TabCompleter(ChatTextEdit* msgEdit_, Group* group_)
    : QObject{msgEdit_}
    , msgEdit{msgEdit_}
    , group{group_}
    , enabled{false}
    , lastCompletionLength{0}
{
}

/* from quassel/src/uisupport/multilineedit.h
    // Compatibility methods with the rest of the classes which still expect this to be a QLineEdit
    inline QString text() const { return toPlainText(); }
    inline QString html() const { return toHtml(); }
    inline int cursorPosition() const { return textCursor().position(); }
    inline void insert(const QString &newText) { insertPlainText(newText); }
    inline void backspace() { keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace,
   Qt::NoModifier)); }
*/

void TabCompleter::buildCompletionList()
{
    // ensure a safe state in case we return early.
    completionMap.clear();
    nextCompletion = completionMap.begin();

    // split the string on the given RE (not chars, nums or braces/brackets) and take the last
    // section
    QString tabAbbrev = msgEdit->toPlainText()
                            .left(msgEdit->textCursor().position())
                            .section(QRegularExpression("[^\\w\\d\\$:@--_\\[\\]{}|`^.\\\\]"), -1, -1);
    // that section is then used as the completion regex
    QRegularExpression regex(QString("^[-_\\[\\]{}|`^.\\\\]*").append(QRegularExpression::escape(tabAbbrev)),
                  QRegularExpression::CaseInsensitiveOption);

    const QString ownNick = group->getSelfName();
    for (const auto& name : group->getPeerList()) {
        if (name == ownNick) {
            continue;   // don't auto complete own name
        }
        if (regex.match(name).hasMatch()) {
            SortableString lower = SortableString(name.toLower());
            completionMap[lower] = name;
        }
    }

    nextCompletion = completionMap.begin();
    lastCompletionLength = tabAbbrev.length();
}


void TabCompleter::complete()
{
    if (!enabled) {
        buildCompletionList();
        enabled = true;
    }

    if (nextCompletion != completionMap.end()) {
        // clear previous completion
        auto cur = msgEdit->textCursor();
        cur.setPosition(cur.selectionEnd());
        msgEdit->setTextCursor(cur);
        for (int i = 0; i < lastCompletionLength; ++i) {
            msgEdit->textCursor().deletePreviousChar();
        }

        // insert completion
        msgEdit->insertPlainText(*nextCompletion);

        // remember charcount to delete next time and advance to next completion
        lastCompletionLength = nextCompletion->length();
        ++nextCompletion;

        // we're completing the first word of the line
        if (msgEdit->textCursor().position() == lastCompletionLength) {
            msgEdit->insertPlainText(nickSuffix);
            lastCompletionLength += nickSuffix.length();
        }
    } else { // we're at the end of the list -> start over again
        if (!completionMap.isEmpty()) {
            nextCompletion = completionMap.begin();
            complete();
        }
    }
}

void TabCompleter::reset()
{
    enabled = false;
}

// this determines the sort order
bool TabCompleter::SortableString::operator<(const SortableString& other) const
{
    /*  QDateTime thisTime = thisUser->lastChannelActivity(_currentBufferId);
    QDateTime thatTime = thatUser->lastChannelActivity(_currentBufferId);


    if (thisTime.isValid() || thatTime.isValid())
        return thisTime > thatTime;
*/ // this could be a
                                                                              // useful feature at
                                                                              // some point

    return QString::localeAwareCompare(contents, other.contents) < 0;
}
