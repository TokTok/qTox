/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2021 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QDate>

class QWidget;

class IFriendListItem
{
public:
    IFriendListItem() = default;
    virtual ~IFriendListItem();
    IFriendListItem(const IFriendListItem&) = default;
    IFriendListItem& operator=(const IFriendListItem&) = default;
    IFriendListItem(IFriendListItem&&) = default;
    IFriendListItem& operator=(IFriendListItem&&) = default;

    virtual bool isFriend() const = 0;
    virtual bool isConference() const = 0;
    virtual bool isOnline() const = 0;
    virtual bool widgetIsVisible() const = 0;
    virtual QString getNameItem() const = 0;
    virtual QDateTime getLastActivity() const = 0;
    virtual QWidget* getWidget() = 0;
    virtual void setWidgetVisible(bool) = 0;

    virtual int getCircleId() const
    {
        return -1;
    }

    int getNameSortedPos() const
    {
        return nameSortedPos;
    }

    void setNameSortedPos(int pos)
    {
        nameSortedPos = pos;
    }

private:
    int nameSortedPos = -1;
};
