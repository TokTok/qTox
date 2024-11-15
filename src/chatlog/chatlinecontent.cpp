/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "chatlinecontent.h"

void ChatLineContent::setIndex(int r, int c)
{
    row = r;
    col = c;
}

int ChatLineContent::getColumn() const
{
    return col;
}

int ChatLineContent::type() const
{
    return GraphicsItemType::ChatLineContentType;
}

void ChatLineContent::selectionMouseMove(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionStarted(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionCleared() {}

void ChatLineContent::selectionDoubleClick(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionTripleClick(QPointF scenePos)
{
    std::ignore = scenePos;
}

void ChatLineContent::selectionFocusChanged(bool focusIn)
{
    std::ignore = focusIn;
}

bool ChatLineContent::isOverSelection(QPointF scenePos) const
{
    std::ignore = scenePos;
    return false;
}

QString ChatLineContent::getSelectedText() const
{
    return QString();
}

void ChatLineContent::fontChanged(const QFont& font)
{
    std::ignore = font;
}

qreal ChatLineContent::getAscent() const
{
    return 0.0;
}

void ChatLineContent::visibilityChanged(bool visible)
{
    std::ignore = visible;
}

void ChatLineContent::reloadTheme() {}

QString ChatLineContent::getText() const
{
    return QString();
}
