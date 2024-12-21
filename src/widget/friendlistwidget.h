/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatitemlayout.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"

#include <QWidget>

class CategoryWidget;
class CircleWidget;
class Core;
class Friend;
class FriendList;
class FriendListManager;
class FriendWidget;
class GenericChatroomWidget;
class ConferenceList;
class ConferenceWidget;
class IFriendListItem;
class IMessageBoxManager;
class Profile;
class QGridLayout;
class QPixmap;
class QVBoxLayout;
class Settings;
class Style;
class Widget;

class FriendListWidget : public QWidget
{
    Q_OBJECT
public:
    using SortingMode = Settings::FriendListSortingMode;
    FriendListWidget(const Core& core, Widget* parent, Settings& settings, Style& style,
                     IMessageBoxManager& messageBoxManager, FriendList& friendList,
                     ConferenceList& conferenceList, Profile& profile, bool conferencesOnTop = true);
    ~FriendListWidget();
    void setMode(SortingMode mode);
    SortingMode getMode() const;

    void addConferenceWidget(ConferenceWidget* widget);
    void addFriendWidget(FriendWidget* w);
    void removeConferenceWidget(ConferenceWidget* w);
    void removeFriendWidget(FriendWidget* w);
    void addCircleWidget(int id);
    void addCircleWidget(FriendWidget* widget = nullptr);
    void removeCircleWidget(CircleWidget* widget);
    void searchChatRooms(const QString& searchString, bool hideOnline = false,
                         bool hideOffline = false, bool hideConferences = false);

    void cycleChats(GenericChatroomWidget* activeChatroomWidget, bool forward);

    void updateActivityTime(const QDateTime& time);

signals:
    void onCompactChanged(bool compact);
    void connectCircleWidget(CircleWidget& circleWidget);
    void searchCircle(CircleWidget& circleWidget);

public slots:
    void renameConferenceWidget(ConferenceWidget* conferenceWidget, const QString& newName);
    void renameCircleWidget(CircleWidget* circleWidget, const QString& newName);
    void onConferencePositionChanged(bool top);
    void moveWidget(FriendWidget* w, Status::Status s, bool add = false);
    void itemsChanged();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void dayTimeout();

private:
    CircleWidget* createCircleWidget(int id = -1);
    CategoryWidget* getTimeCategoryWidget(const Friend* frd) const;
    void sortByMode();
    void cleanMainLayout();
    QWidget* getNextWidgetForName(IFriendListItem* currentPos, bool forward) const;
    QVector<std::shared_ptr<IFriendListItem>> getItemsFromCircle(CircleWidget* circle) const;

    SortingMode mode;
    QVBoxLayout* listLayout = nullptr;
    QVBoxLayout* activityLayout = nullptr;
    QTimer* dayTimer;
    FriendListManager* manager;

    const Core& core;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
    FriendList& friendList;
    ConferenceList& conferenceList;
    Profile& profile;
};
