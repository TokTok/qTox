/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2021 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "ifriendlistitem.h"

#include <QObject>
#include <QVector>

#include <memory>

class FriendListManager : public QObject
{
    Q_OBJECT
public:
    using IFriendListItemPtr = std::shared_ptr<IFriendListItem>;

    explicit FriendListManager(int countContacts_, QObject* parent = nullptr);

    QVector<IFriendListItemPtr> getItems() const;
    bool needHideCircles() const;
    // If the contact positions have changed, need to redraw view
    bool getPositionsChanged() const;
    bool getConferencesOnTop() const;

    void addFriendListItem(IFriendListItem* item);
    void removeFriendListItem(IFriendListItem* item);
    void sortByName();
    void sortByActivity();
    void resetParents();
    void setFilter(const QString& searchString, bool hideOnline, bool hideOffline, bool hideConferences);
    void applyFilter();
    void updatePositions();
    void setSortRequired();

    void setConferencesOnTop(bool v);

signals:
    void itemsChanged();

private:
    struct FilterParams
    {
        QString searchString = "";
        bool hideOnline = false;
        bool hideOffline = false;
        bool hideConferences = false;
    } filterParams;

    void removeAll(IFriendListItem* item);
    bool cmpByName(const IFriendListItemPtr& itemA, const IFriendListItemPtr& itemB);
    bool cmpByActivity(const IFriendListItemPtr& itemA, const IFriendListItemPtr& itemB);

    bool byName = true;
    bool hideCircles = false;
    bool conferencesOnTop = true;
    bool positionsChanged = false;
    bool needSort = false;
    QVector<IFriendListItemPtr> items;
    // At startup, while the size of items is less than countContacts, the view will not be processed to improve performance
    int countContacts = 0;
};
