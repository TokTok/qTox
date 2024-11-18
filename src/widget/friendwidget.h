/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatroomwidget.h"
#include "src/core/toxpk.h"
#include "src/model/friendlist/ifriendlistitem.h"

#include <memory>

class FriendChatroom;
class QPixmap;
class MaskablePixmapWidget;
class CircleWidget;
class Settings;
class Style;
class IMessageBoxManager;
class Profile;

class FriendWidget : public GenericChatroomWidget, public IFriendListItem
{
    Q_OBJECT
public:
    FriendWidget(std::shared_ptr<FriendChatroom> chatroom, bool compact_, Settings& settings,
                 Style& style, IMessageBoxManager& messageBoxManager, Profile& profile);

    void contextMenuEvent(QContextMenuEvent* event) final;
    void setAsActiveChatroom() final;
    void setAsInactiveChatroom() final;
    void updateStatusLight() final;
    void resetEventFlags() final;
    QString getStatusString() const final;
    const Friend* getFriend() const final;
    const Chat* getChat() const final;

    bool isFriend() const final;
    bool isConference() const final;
    bool isOnline() const final;
    bool widgetIsVisible() const final;
    QString getNameItem() const final;
    QDateTime getLastActivity() const final;
    int getCircleId() const final;
    QWidget* getWidget() final;
    void setWidgetVisible(bool visible) final;

signals:
    void friendWidgetClicked(FriendWidget* widget);
    void removeFriend(const ToxPk& friendPk);
    void copyFriendIdToClipboard(const ToxPk& friendPk);
    void contextMenuCalled(QContextMenuEvent* event);
    void friendHistoryRemoved();
    void updateFriendActivity(Friend& frnd);

public slots:
    void onAvatarSet(const ToxPk& friendPk, const QPixmap& pic);
    void onAvatarRemoved(const ToxPk& friendPk);
    void onContextMenuCalled(QContextMenuEvent* event);
    void setActive(bool active_);

protected:
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void setFriendAlias();

private slots:
    void removeChatWindow();
    void moveToNewCircle();
    void removeFromCircle();
    void moveToCircle(int circleId);
    void changeAutoAccept(bool enable);
    void showDetails();

public:
    std::shared_ptr<FriendChatroom> chatroom;
    bool isDefaultAvatar;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
    Profile& profile;
};
