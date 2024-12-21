/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QPointF>
#include <QRectF>
#include <QVector>
#include <memory>

class ChatWidget;
class ChatLineContent;
class QGraphicsScene;
class QStyleOptionGraphicsItem;
class QFont;

struct ColumnFormat
{
    enum Policy
    {
        FixedSize,
        VariableSize,
    };

    enum Align
    {
        Left,
        Center,
        Right,
    };

    ColumnFormat() {}
    ColumnFormat(qreal s, Policy p, Align hAlign_ = Left)
        : size(s)
        , policy(p)
        , hAlign(hAlign_)
    {
    }

    qreal size = 1.0;
    Policy policy = VariableSize;
    Align hAlign = Left;
};

using ColumnFormats = QVector<ColumnFormat>;

class ChatLine
{
public:
    using Ptr = std::shared_ptr<ChatLine>;

    ChatLine();
    virtual ~ChatLine();

    QRectF sceneBoundingRect() const;

    void replaceContent(int col, ChatLineContent* lineContent);
    void layout(qreal width, QPointF scenePos);
    void moveBy(qreal deltaY);
    void removeFromScene();
    void addToScene(QGraphicsScene* scene);
    void setVisible(bool visible);
    void selectionCleared();
    void selectionFocusChanged(bool focusIn);
    void fontChanged(const QFont& font);
    void reloadTheme();

    int getColumnCount();

    ChatLineContent* getContent(int col) const;
    ChatLineContent* getContent(QPointF scenePos) const;

    bool isOverSelection(QPointF scenePos);

    // comparators
    static bool lessThanBSRectTop(const ChatLine::Ptr& lhs, const qreal& rhs);
    static bool lessThanBSRectBottom(const ChatLine::Ptr& lhs, const qreal& rhs);

protected:
    friend class ChatWidget;

    QPointF mapToContent(ChatLineContent* c, QPointF pos);

    void addColumn(ChatLineContent* item, ColumnFormat fmt);
    void updateBBox();
    void visibilityChanged(bool visible);

private:
    int row = -1;
    QVector<ChatLineContent*> content;
    QVector<ColumnFormat> format;
    qreal width = 100.0;
    qreal columnSpacing = 15.0;
    QRectF bbox;
    bool isVisible = false;
};
