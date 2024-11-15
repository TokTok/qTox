/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "categorywidget.h"

class ContentDialog;
class Core;
class Settings;
class Style;
class IMessageBoxManager;
class FriendList;
class GroupList;
class Profile;

class CircleWidget final : public CategoryWidget
{
    Q_OBJECT
public:
    CircleWidget(const Core& core_, FriendListWidget* parent, int id_, Settings& settings,
        Style& style, IMessageBoxManager& messageboxManager, FriendList& friendList,
        GroupList& groupList, Profile& profile);
    ~CircleWidget();

    void editName();
    static CircleWidget* getFromID(int id);

signals:
    void renameRequested(CircleWidget* circleWidget, const QString& newName);
    void newContentDialog(ContentDialog& contentDialog);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final;
    void dragEnterEvent(QDragEnterEvent* event) final;
    void dragLeaveEvent(QDragLeaveEvent* event) final;
    void dropEvent(QDropEvent* event) final;

private:
    void onSetName() final;
    void onExpand() final;
    void onAddFriendWidget(FriendWidget* w) final;
    void updateID(int index);

    static QHash<int, CircleWidget*> circleList;
    int id;

    const Core& core;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
    FriendList& friendList;
    GroupList& groupList;
    Profile& profile;
};
