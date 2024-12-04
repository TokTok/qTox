/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "chatlinecontentproxy.h"
#include "src/chatlog/content/filetransferwidget.h"
#include <QDebug>
#include <QLayout>
#include <QPainter>
#include <QWidget>

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, ChatLineContentProxyType type,
                                           int minWidth, float widthInPercent)
    : widthPercent(widthInPercent)
    , widthMin(minWidth)
    , widgetType{type}
{
    proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
}

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent)
    : ChatLineContentProxy(widget, GenericType, minWidth, widthInPercent)
{
}

ChatLineContentProxy::ChatLineContentProxy(FileTransferWidget* widget, int minWidth, float widthInPercent)
    : ChatLineContentProxy(widget, FileTransferWidgetType, minWidth, widthInPercent)
{
}

QRectF ChatLineContentProxy::boundingRect() const
{
    QRectF result = proxy->boundingRect();
    result.setHeight(result.height() + 5);
    return result;
}

void ChatLineContentProxy::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                 QWidget* widget)
{
    painter->setClipRect(boundingRect());
    proxy->paint(painter, option, widget);
}

qreal ChatLineContentProxy::getAscent() const
{
    return 7.0;
}

QWidget* ChatLineContentProxy::getWidget() const
{
    return proxy->widget();
}

void ChatLineContentProxy::setWidth(float width)
{
    prepareGeometryChange();
    proxy->widget()->setFixedWidth(qMax(static_cast<int>(width * widthPercent), widthMin));
}

ChatLineContentProxy::ChatLineContentProxyType ChatLineContentProxy::getWidgetType() const
{
    return widgetType;
}
