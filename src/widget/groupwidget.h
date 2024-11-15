/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatroomwidget.h"

#include "src/model/chatroom/groupchatroom.h"
#include "src/model/friendlist/ifriendlistitem.h"
#include "src/core/groupid.h"

#include <memory>

class Settings;
class Style;

class GroupWidget final : public GenericChatroomWidget, public IFriendListItem
{
    Q_OBJECT
public:
    GroupWidget(std::shared_ptr<GroupChatroom> chatroom_, bool compact, Settings& settings,
        Style& style);
    ~GroupWidget();
    void setAsInactiveChatroom() final;
    void setAsActiveChatroom() final;
    void updateStatusLight() final;
    void resetEventFlags() final;
    QString getStatusString() const final;
    Group* getGroup() const final;
    const Chat* getChat() const final;
    void setName(const QString& name);
    void editName();

    bool isFriend() const final;
    bool isGroup() const final;
    QString getNameItem() const final;
    bool isOnline() const final;
    bool widgetIsVisible() const final;
    QDateTime getLastActivity() const final;
    QWidget* getWidget() final;
    void setWidgetVisible(bool visible) final;

signals:
    void groupWidgetClicked(GroupWidget* widget);
    void removeGroup(const GroupId& groupId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final;
    void mousePressEvent(QMouseEvent* event) final;
    void mouseMoveEvent(QMouseEvent* event) final;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dragLeaveEvent(QDragLeaveEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;

private slots:
    void retranslateUi();
    void updateTitle(const QString& author, const QString& newName);
    void updateUserCount(int numPeers);

public:
    GroupId groupId;

private:
    std::shared_ptr<GroupChatroom> chatroom;
};
