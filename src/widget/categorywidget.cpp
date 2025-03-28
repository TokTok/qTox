/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "categorywidget.h"

#include "friendlistlayout.h"
#include "friendlistwidget.h"
#include "friendwidget.h"

#include "src/model/status.h"
#include "src/widget/style.h"
#include "tool/croppinglabel.h"

#include <QApplication>
#include <QBoxLayout>
#include <QMouseEvent>

void CategoryWidget::emitChatroomWidget(QLayout* layout, int index)
{
    QWidget* widget = layout->itemAt(index)->widget();
    auto* chatWidget = qobject_cast<GenericChatroomWidget*>(widget);
    if (chatWidget != nullptr) {
        emit chatWidget->chatroomWidgetClicked(chatWidget);
    }
}

CategoryWidget::CategoryWidget(bool compact_, Settings& settings_, Style& style_, QWidget* parent)
    : GenericChatItemWidget(compact_, style_, parent)
    , settings{settings_}
    , style{style_}
{
    container = new QWidget(this);
    container->setObjectName("circleWidgetContainer");
    container->setLayoutDirection(Qt::LeftToRight);

    statusLabel = new QLabel(this);
    statusLabel->setObjectName("status");
    statusLabel->setTextFormat(Qt::PlainText);

    statusPic.setPixmap(QPixmap(style.getImagePath("chatArea/scrollBarRightArrow.svg", settings)));

    fullLayout = new QVBoxLayout(this);
    fullLayout->setSpacing(0);
    fullLayout->setContentsMargins(0, 0, 0, 0);
    fullLayout->addWidget(container);

    lineFrame = new QFrame(container);
    lineFrame->setObjectName("line");
    lineFrame->setFrameShape(QFrame::HLine);
    lineFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    lineFrame->resize(0, 0);

    listLayout = new FriendListLayout();
    listWidget = new QWidget(this);
    listWidget->setLayout(listLayout);
    fullLayout->addWidget(listWidget);

    setAcceptDrops(true);

    onCompactChanged(isCompact());

    setExpanded(true, false);
    updateStatus();
}

bool CategoryWidget::isExpanded() const
{
    return expanded;
}

void CategoryWidget::setExpanded(bool isExpanded, bool save)
{
    if (expanded == isExpanded) {
        return;
    }
    expanded = isExpanded;
    setMouseTracking(true);

    // The listWidget will receive a enterEvent for some reason if now visible.
    // Using the following, we prevent that.
    listWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    listWidget->setVisible(isExpanded);
    listWidget->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    QString pixmapPath;
    if (isExpanded)
        pixmapPath = style.getImagePath("chatArea/scrollBarDownArrow.svg", settings);
    else
        pixmapPath = style.getImagePath("chatArea/scrollBarRightArrow.svg", settings);
    statusPic.setPixmap(QPixmap(pixmapPath));

    if (save)
        onExpand();
}

void CategoryWidget::leaveEvent(QEvent* event)
{
    event->ignore();
}

void CategoryWidget::setName(const QString& name, bool save)
{
    nameLabel->setText(name);

    if (isCompact())
        nameLabel->minimizeMaximumWidth();

    if (save)
        onSetName();
}

void CategoryWidget::editName()
{
    nameLabel->editBegin();
    nameLabel->setMaximumWidth(QWIDGETSIZE_MAX);
}

void CategoryWidget::addFriendWidget(FriendWidget* w, Status::Status s)
{
    listLayout->addFriendWidget(w, s);
    updateStatus();
    onAddFriendWidget(w);
    w->reloadTheme(); // Otherwise theme will change when moving to another circle.
}

void CategoryWidget::removeFriendWidget(FriendWidget* w, Status::Status s)
{
    listLayout->removeFriendWidget(w, s);
    updateStatus();
}

void CategoryWidget::updateStatus()
{
    const QString online = QString::number(listLayout->friendOnlineCount());
    const QString offline = QString::number(listLayout->friendTotalCount());
    const QString text = online + QStringLiteral(" / ") + offline;
    statusLabel->setText(text);
}

bool CategoryWidget::hasChatRooms() const
{
    return listLayout->hasChatRooms();
}

void CategoryWidget::search(const QString& searchString, bool updateAll, bool hideOnline,
                            bool hideOffline)
{
    if (updateAll) {
        listLayout->searchChatRooms(searchString, hideOnline, hideOffline);
    }
    const bool inCategory = searchString.isEmpty() && !(hideOnline && hideOffline);
    setVisible(inCategory || listLayout->hasChatRooms());
}

bool CategoryWidget::cycleChats(bool forward)
{
    if (listLayout->friendTotalCount() == 0) {
        return false;
    }
    if (forward) {
        if (!listLayout->getLayoutOnline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(), 0);
            return true;
        }
        if (!listLayout->getLayoutOffline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(), 0);
            return true;
        }
    } else {
        if (!listLayout->getLayoutOffline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOffline(),
                               listLayout->getLayoutOffline()->count() - 1);
            return true;
        }
        if (!listLayout->getLayoutOnline()->isEmpty()) {
            setExpanded(true);
            emitChatroomWidget(listLayout->getLayoutOnline(),
                               listLayout->getLayoutOnline()->count() - 1);
            return true;
        }
    }
    return false;
}

bool CategoryWidget::cycleChats(FriendWidget* activeChatroomWidget, bool forward)
{
    int index = -1;
    QLayout* currentLayout = nullptr;

    auto* friendWidget = qobject_cast<FriendWidget*>(activeChatroomWidget);
    if (friendWidget == nullptr)
        return false;

    currentLayout = listLayout->getLayoutOnline();
    index = listLayout->indexOfFriendWidget(friendWidget, true);
    if (index == -1) {
        currentLayout = listLayout->getLayoutOffline();
        index = listLayout->indexOfFriendWidget(friendWidget, false);
    }

    index += forward ? 1 : -1;
    for (;;) {
        // Bounds checking.
        if (index < 0) {
            if (currentLayout == listLayout->getLayoutOffline())
                currentLayout = listLayout->getLayoutOnline();
            else
                return false;

            index = currentLayout->count() - 1;
            continue;
        }
        if (index >= currentLayout->count()) {
            if (currentLayout == listLayout->getLayoutOnline())
                currentLayout = listLayout->getLayoutOffline();
            else
                return false;

            index = 0;
            continue;
        }

        auto* chatWidget =
            qobject_cast<GenericChatroomWidget*>(currentLayout->itemAt(index)->widget());
        if (chatWidget != nullptr)
            emit chatWidget->chatroomWidgetClicked(chatWidget);
        return true;
    }
}

void CategoryWidget::onCompactChanged(bool _compact)
{
    delete topLayout;
    delete mainLayout;

    topLayout = new QHBoxLayout;
    topLayout->setSpacing(0);
    topLayout->setContentsMargins(0, 0, 0, 0);

    std::ignore = _compact;
    setCompact(true);

    nameLabel->minimizeMaximumWidth();

    mainLayout = nullptr;

    container->setFixedHeight(25);
    container->setLayout(topLayout);

    topLayout->addSpacing(18);
    topLayout->addWidget(&statusPic);
    topLayout->addSpacing(5);
    topLayout->addWidget(nameLabel, 100);
    topLayout->addWidget(lineFrame, 1);
    topLayout->addSpacing(5);
    topLayout->addWidget(statusLabel);
    topLayout->addSpacing(5);
    topLayout->activate();

    Style::repolish(this);
}

void CategoryWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        setExpanded(!expanded);
}

void CategoryWidget::setContainerAttribute(Qt::WidgetAttribute attribute, bool enabled)
{
    container->setAttribute(attribute, enabled);
    Style::repolish(container);
}

QLayout* CategoryWidget::friendOfflineLayout() const
{
    return listLayout->getLayoutOffline();
}

QLayout* CategoryWidget::friendOnlineLayout() const
{
    return listLayout->getLayoutOnline();
}

void CategoryWidget::moveFriendWidgets(FriendListWidget* friendList)
{
    listLayout->moveFriendWidgets(friendList);
}
