/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2021 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "friendlistmanager.h"

#include <QWidget>

namespace {

/**
 * @brief Checks whether the item is a group chat, i.e. either a legacy conference
 * (toxconferences) or a new group chat (toxgroups, "NGC").
 * Both kinds are kept together at the top of the contact list and sorted by name.
 */
bool isGroupChat(const IFriendListItem& item)
{
    return item.isConference() || item.isGroup();
}

} // namespace

FriendListManager::FriendListManager(int countContacts_, QObject* parent)
    : QObject(parent)
{
    countContacts = countContacts_;
}

QVector<FriendListManager::IFriendListItemPtr> FriendListManager::getItems() const
{
    return items;
}

bool FriendListManager::needHideCircles() const
{
    return hideCircles;
}

bool FriendListManager::getPositionsChanged() const
{
    return positionsChanged;
}

bool FriendListManager::getConferencesOnTop() const
{
    return conferencesOnTop;
}

void FriendListManager::addFriendListItem(IFriendListItem* item)
{
    if (item->isConference() && item->getWidget() != nullptr) {
        items.push_back(IFriendListItemPtr(item, [](IFriendListItem* conferenceItem) {
            conferenceItem->getWidget()->deleteLater();
        }));
    } else {
        items.push_back(IFriendListItemPtr(item));
    }

    if (countContacts <= items.size()) {
        countContacts = 0;
        setSortRequired();
    }
}

void FriendListManager::removeFriendListItem(IFriendListItem* item)
{
    removeAll(item);
    setSortRequired();
}

void FriendListManager::sortByName()
{
    byName = true;
    updatePositions();
}

void FriendListManager::sortByActivity()
{
    byName = false;
    updatePositions();
}

void FriendListManager::resetParents()
{
    for (auto& item : items) {
        IFriendListItem* itemTmp = item.get();
        itemTmp->getWidget()->setParent(nullptr);
    }
}

void FriendListManager::setFilter(const QString& searchString, bool hideOnline, bool hideOffline,
                                  bool hideConferences, bool hideGroups)
{
    if (filterParams.searchString == searchString && filterParams.hideOnline == hideOnline
        && filterParams.hideOffline == hideOffline && filterParams.hideConferences == hideConferences
        && filterParams.hideGroups == hideGroups) {
        return;
    }
    filterParams.searchString = searchString;
    filterParams.hideOnline = hideOnline;
    filterParams.hideOffline = hideOffline;
    filterParams.hideConferences = hideConferences;
    filterParams.hideGroups = hideGroups;

    setSortRequired();
}

void FriendListManager::applyFilter()
{
    const QString searchString = filterParams.searchString;

    for (const IFriendListItemPtr& itemTmp : items) {
        if (searchString.isEmpty()) {
            itemTmp->setWidgetVisible(true);
        } else {
            const QString tmp_name = itemTmp->getNameItem();
            itemTmp->setWidgetVisible(tmp_name.contains(searchString, Qt::CaseInsensitive));
        }

        if (filterParams.hideOnline && itemTmp->isOnline()) {
            if (itemTmp->isFriend()) {
                itemTmp->setWidgetVisible(false);
            }
        }

        if (filterParams.hideOffline && !itemTmp->isOnline()) {
            itemTmp->setWidgetVisible(false);
        }

        if (filterParams.hideConferences && itemTmp->isConference()) {
            itemTmp->setWidgetVisible(false);
        }

        if (filterParams.hideGroups && itemTmp->isGroup()) {
            itemTmp->setWidgetVisible(false);
        }
    }

    hideCircles = filterParams.hideOnline && filterParams.hideOffline;
}

void FriendListManager::updatePositions()
{
    positionsChanged = true;

    if (byName) {
        auto sortName = [&](const IFriendListItemPtr& a, const IFriendListItemPtr& b) {
            return cmpByName(a, b);
        };
        if (!needSort) {
            if (std::is_sorted(items.begin(), items.end(), sortName)) {
                positionsChanged = false;
                return;
            }
        }
        std::sort(items.begin(), items.end(), sortName);

    } else {
        auto sortActivity = [&](const IFriendListItemPtr& a, const IFriendListItemPtr& b) {
            return cmpByActivity(a, b);
        };
        if (!needSort) {
            if (std::is_sorted(items.begin(), items.end(), sortActivity)) {
                positionsChanged = false;
                return;
            }
        }
        std::sort(items.begin(), items.end(), sortActivity);
    }

    needSort = false;
}

void FriendListManager::setSortRequired()
{
    needSort = true;
    emit itemsChanged();
}

void FriendListManager::setConferencesOnTop(bool v)
{
    conferencesOnTop = v;
}

void FriendListManager::removeAll(IFriendListItem* item)
{
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].get() == item) {
            items.remove(i);
            --i;
        }
    }
}

bool FriendListManager::cmpByName(const IFriendListItemPtr& a, const IFriendListItemPtr& b) const
{
    const bool aIsGroupChat = isGroupChat(*a);
    const bool bIsGroupChat = isGroupChat(*b);

    if (aIsGroupChat != bIsGroupChat) {
        if (conferencesOnTop) {
            return aIsGroupChat;
        }
        // Group chats are placed below online friends, but above offline ones.
        return aIsGroupChat ? !b->isOnline() : a->isOnline();
    }

    if (aIsGroupChat) {
        return a->getNameItem().toUpper() < b->getNameItem().toUpper();
    }

    if (a->isOnline() && !b->isOnline()) {
        return true;
    }

    if (!a->isOnline() && b->isOnline()) {
        return false;
    }

    return a->getNameItem().toUpper() < b->getNameItem().toUpper();
}

bool FriendListManager::cmpByActivity(const IFriendListItemPtr& a, const IFriendListItemPtr& b)
{
    const bool aIsGroupChat = isGroupChat(*a);
    const bool bIsGroupChat = isGroupChat(*b);

    if (aIsGroupChat != bIsGroupChat) {
        return aIsGroupChat;
    }

    if (aIsGroupChat) {
        return a->getNameItem().toUpper() < b->getNameItem().toUpper();
    }

    const QDateTime dateA = a->getLastActivity();
    const QDateTime dateB = b->getLastActivity();
    if (dateA.date() == dateB.date()) {
        if (a->isOnline() && !b->isOnline()) {
            return true;
        }

        if (!a->isOnline() && b->isOnline()) {
            return false;
        }
        return a->getNameItem().toUpper() < b->getNameItem().toUpper();
    }

    return a->getLastActivity() > b->getLastActivity();
}
