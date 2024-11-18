/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include <QBoxLayout>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QVariant>

#include <cassert>

#include "circlewidget.h"
#include "contentdialog.h"
#include "friendlistwidget.h"
#include "friendwidget.h"
#include "widget.h"
#include "tool/croppinglabel.h"

#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include "src/widget/form/chatform.h"

QHash<int, CircleWidget*> CircleWidget::circleList;

CircleWidget::CircleWidget(const Core& core_, FriendListWidget* parent, int id_, Settings& settings_,
                           Style& style_, IMessageBoxManager& messageBoxManager_,
                           FriendList& friendList_, ConferenceList& conferenceList_, Profile& profile_)
    : CategoryWidget(settings_.getCompactLayout(), settings_, style_, parent)
    , id(id_)
    , core{core_}
    , settings{settings_}
    , style{style_}
    , messageBoxManager{messageBoxManager_}
    , friendList{friendList_}
    , conferenceList{conferenceList_}
    , profile{profile_}
{
    setName(settings.getCircleName(id), false);
    circleList[id] = this;

    connect(nameLabel, &CroppingLabel::editFinished, this, [this](const QString& newName) {
        if (!newName.isEmpty())
            emit renameRequested(this, newName);
    });

    connect(nameLabel, &CroppingLabel::editRemoved, this, [this]() {
        if (isCompact())
            nameLabel->minimizeMaximumWidth();
    });

    setExpanded(settings.getCircleExpanded(id), false);
    updateStatus();
}

CircleWidget::~CircleWidget()
{
    if (circleList[id] == this)
        circleList.remove(id);
}

void CircleWidget::editName()
{
    CategoryWidget::editName();
}

CircleWidget* CircleWidget::getFromID(int id)
{
    auto circleIt = circleList.find(id);

    if (circleIt != circleList.end())
        return circleIt.value();

    return nullptr;
}

void CircleWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu;
    QAction* renameAction = menu.addAction(tr("Rename circle", "Menu for renaming a circle"));
    QAction* removeAction = menu.addAction(tr("Remove circle", "Menu for removing a circle"));
    QAction* openAction = nullptr;

    if (friendOfflineLayout()->count() + friendOnlineLayout()->count() > 0)
        openAction = menu.addAction(tr("Open all in new window"));

    QAction* selectedItem = menu.exec(mapToGlobal(event->pos()));

    if (selectedItem != nullptr) {
        if (selectedItem == renameAction) {
            editName();
        } else if (selectedItem == removeAction) {
            FriendListWidget* friendListWidget = static_cast<FriendListWidget*>(parentWidget());
            moveFriendWidgets(friendListWidget);

            friendListWidget->removeCircleWidget(this);

            const int replacedCircle = settings.removeCircle(id);

            auto circleReplace = circleList.find(replacedCircle);
            if (circleReplace != circleList.end())
                circleReplace.value()->updateID(id);
            else
                assert(true); // This should never happen.

            circleList.remove(replacedCircle);
        } else if (selectedItem == openAction) {
            ContentDialog* dialog = new ContentDialog(core, settings, style, messageBoxManager,
                                                      friendList, conferenceList, profile);
            emit newContentDialog(*dialog);
            for (int i = 0; i < friendOnlineLayout()->count(); ++i) {
                QWidget* const widget = friendOnlineLayout()->itemAt(i)->widget();
                FriendWidget* const friendWidget = qobject_cast<FriendWidget*>(widget);

                if (friendWidget != nullptr) {
                    friendWidget->activate();
                }
            }
            for (int i = 0; i < friendOfflineLayout()->count(); ++i) {
                QWidget* const widget = friendOfflineLayout()->itemAt(i)->widget();
                FriendWidget* const friendWidget = qobject_cast<FriendWidget*>(widget);

                if (friendWidget != nullptr) {
                    friendWidget->activate();
                }
            }

            dialog->show();
            dialog->ensureSplitterVisible();
        }
    }

    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk toxPk(event->mimeData()->data("toxPk"));
    Friend* f = friendList.findFriend(toxPk);
    if (f != nullptr)
        event->acceptProposedAction();

    setContainerAttribute(Qt::WA_UnderMouse, true); // Simulate hover.
}

void CircleWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    std::ignore = event;
    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::dropEvent(QDropEvent* event)
{
    setExpanded(true, false);

    // Check, that the element is dropped from qTox
    QObject* o = event->source();
    FriendWidget* widget = qobject_cast<FriendWidget*>(o);
    if (widget == nullptr)
        return;

    if (!event->mimeData()->hasFormat("toxPk")) {
        return;
    }
    // Check, that the user has a friend with the same ToxId
    const ToxPk toxPk{event->mimeData()->data("toxPk")};
    Friend* f = friendList.findFriend(toxPk);
    if (f == nullptr)
        return;

    // Save CircleWidget before changing the Id
    const int circleId = settings.getFriendCircleID(toxPk);
    CircleWidget* circleWidget = getFromID(circleId);

    addFriendWidget(widget, f->getStatus());
    settings.savePersonal();

    if (circleWidget != nullptr) {
        circleWidget->updateStatus();
    }

    setContainerAttribute(Qt::WA_UnderMouse, false);
}

void CircleWidget::onSetName()
{
    settings.setCircleName(id, getName());
}

void CircleWidget::onExpand()
{
    settings.setCircleExpanded(id, isExpanded());
    settings.savePersonal();
}

void CircleWidget::onAddFriendWidget(FriendWidget* w)
{
    const Friend* f = w->getFriend();
    const ToxPk toxId = f->getPublicKey();
    settings.setFriendCircleID(toxId, id);
}

void CircleWidget::updateID(int index)
{
    // For when a circle gets destroyed, another takes its id.
    // This function updates all friends widgets for this new id.

    if (id == index) {
        return;
    }

    id = index;
    circleList[id] = this;

    for (int i = 0; i < friendOnlineLayout()->count(); ++i) {
        const QWidget* w = friendOnlineLayout()->itemAt(i)->widget();
        const FriendWidget* friendWidget = qobject_cast<const FriendWidget*>(w);

        if (friendWidget != nullptr) {
            const Friend* f = friendWidget->getFriend();
            settings.setFriendCircleID(f->getPublicKey(), id);
        }
    }

    for (int i = 0; i < friendOfflineLayout()->count(); ++i) {
        const QWidget* w = friendOfflineLayout()->itemAt(i)->widget();
        const FriendWidget* friendWidget = qobject_cast<const FriendWidget*>(w);

        if (friendWidget != nullptr) {
            const Friend* f = friendWidget->getFriend();
            settings.setFriendCircleID(f->getPublicKey(), id);
        }
    }
}
