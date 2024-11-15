/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatitemlayout.h"
#include "src/model/status.h"
#include "src/core/core.h"
#include <QBoxLayout>

class FriendWidget;
class FriendListWidget;

class FriendListLayout : public QVBoxLayout
{
    Q_OBJECT
public:
    explicit FriendListLayout();
    explicit FriendListLayout(QWidget* parent);

    void addFriendWidget(FriendWidget* widget, Status::Status s);
    void removeFriendWidget(FriendWidget* widget, Status::Status s);
    int indexOfFriendWidget(GenericChatItemWidget* widget, bool online) const;
    void moveFriendWidgets(FriendListWidget* listWidget);
    int friendOnlineCount() const;
    int friendTotalCount() const;

    bool hasChatrooms() const;
    void searchChatrooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false);

    QLayout* getLayoutOnline() const;
    QLayout* getLayoutOffline() const;

private:
    void init();
    QLayout* getFriendLayout(Status::Status s) const;

    GenericChatItemLayout friendOnlineLayout;
    GenericChatItemLayout friendOfflineLayout;
};
