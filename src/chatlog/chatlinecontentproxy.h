/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "chatlinecontent.h"
#include <QGraphicsProxyWidget>

class FileTransferWidget;

class ChatLineContentProxy : public ChatLineContent
{
    Q_OBJECT

public:
    /**
     * @brief Type tag to avoid dynamic_cast of contained QWidget*
     */
    enum ChatLineContentProxyType
    {
        GenericType = 0,
        FileTransferWidgetType = 0,
    };

public:
    ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent = 1.0f);
    ChatLineContentProxy(FileTransferWidget* widget, int minWidth, float widthInPercent = 1.0f);

    QRectF boundingRect() const override;
    void setWidth(float width) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    qreal getAscent() const override;

    QWidget* getWidget() const;
    ChatLineContentProxyType getWidgetType() const;

protected:
    ChatLineContentProxy(QWidget* widget, ChatLineContentProxyType type, int minWidth,
                         float widthInPercent);

private:
    QGraphicsProxyWidget* proxy;
    float widthPercent;
    int widthMin;
    const ChatLineContentProxyType widgetType;
};
