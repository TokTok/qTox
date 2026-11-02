/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "genericchatroomwidget.h"

#include "src/core/groupid.h"
#include "src/model/chatroom/grouproom.h"
#include "src/model/friendlist/ifriendlistitem.h"

#include <memory>

class Settings;
class Style;

class GroupWidget final : public GenericChatroomWidget, public IFriendListItem
{
    Q_OBJECT
public:
    GroupWidget(std::shared_ptr<GroupRoom> chatroom_, bool compact, Settings& settings, Style& style,
                QWidget* parent);
    ~GroupWidget() override;
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
    bool isConference() const final;
    bool isGroup() const final;
    QString getNameItem() const final;
    bool isOnline() const final;
    void startCall() final;
    void stopCall() final;
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

private:
    std::shared_ptr<GroupRoom> chatroom;

public:
    GroupId groupId;
};
