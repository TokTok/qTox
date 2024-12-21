/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatitemwidget.h"
#include "src/model/status.h"

class FriendListLayout;
class FriendListWidget;
class FriendWidget;
class QVBoxLayout;
class QHBoxLayout;
class Settings;
class Style;

class CategoryWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    CategoryWidget(bool compact_, Settings& settings, Style& style, QWidget* parent = nullptr);

    bool isExpanded() const;
    void setExpanded(bool isExpanded, bool save = true);
    void setName(const QString& name, bool save = true);

    void addFriendWidget(FriendWidget* w, Status::Status s);
    void removeFriendWidget(FriendWidget* w, Status::Status s);
    void updateStatus();

    bool hasChatRooms() const;
    bool cycleChats(bool forward);
    bool cycleChats(FriendWidget* activeChatroomWidget, bool forward);
    void search(const QString& searchString, bool updateAll = false, bool hideOnline = false,
                bool hideOffline = false);

public slots:
    void onCompactChanged(bool compact);
    void moveFriendWidgets(FriendListWidget* friendList);

protected:
    void leaveEvent(QEvent* event) final;
    void mouseReleaseEvent(QMouseEvent* event) final;

    void editName();
    void setContainerAttribute(Qt::WidgetAttribute attribute, bool enabled);
    QLayout* friendOnlineLayout() const;
    QLayout* friendOfflineLayout() const;
    void emitChatroomWidget(QLayout* layout, int index);

private:
    virtual void onSetName() {}
    virtual void onExpand() {}
    virtual void onAddFriendWidget(FriendWidget* widget)
    {
        std::ignore = widget;
    }

    QWidget* listWidget;
    FriendListLayout* listLayout;
    QVBoxLayout* fullLayout;
    QVBoxLayout* mainLayout = nullptr;
    QHBoxLayout* topLayout = nullptr;
    QLabel* statusLabel;
    QWidget* container;
    QFrame* lineFrame;
    bool expanded = false;
    Settings& settings;
    Style& style;
};
