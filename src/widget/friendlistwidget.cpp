/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "friendlistwidget.h"
#include "circlewidget.h"
#include "conferencewidget.h"
#include "friendwidget.h"
#include "widget.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/model/friendlist/friendlistmanager.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/categorywidget.h"

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QGridLayout>
#include <QMimeData>
#include <QTimer>
#include <cassert>

namespace {
enum class Time
{
    Today,
    Yesterday,
    ThisWeek,
    ThisMonth,
    Month1Ago,
    Month2Ago,
    Month3Ago,
    Month4Ago,
    Month5Ago,
    LongAgo,
    Never
};

const int LAST_TIME = static_cast<int>(Time::Never);

Time getTimeBucket(const QDateTime& date)
{
    if (date == QDateTime()) {
        return Time::Never;
    }

    const QDate today = QDate::currentDate();
    // clang-format off
    const QMap<Time, QDate> dates {
        { Time::Today,     today.addDays(0)    },
        { Time::Yesterday, today.addDays(-1)   },
        { Time::ThisWeek,  today.addDays(-6)   },
        { Time::ThisMonth, today.addMonths(-1) },
        { Time::Month1Ago, today.addMonths(-2) },
        { Time::Month2Ago, today.addMonths(-3) },
        { Time::Month3Ago, today.addMonths(-4) },
        { Time::Month4Ago, today.addMonths(-5) },
        { Time::Month5Ago, today.addMonths(-6) },
    };
    // clang-format on

    for (const Time time : dates.keys()) {
        if (dates[time] <= date.date()) {
            return time;
        }
    }

    return Time::LongAgo;
}

QDateTime getActiveTimeFriend(const Friend* contact, Settings& settings)
{
    return settings.getFriendActivity(contact->getPublicKey());
}

qint64 timeUntilTomorrow()
{
    const QDateTime now = QDateTime::currentDateTime();
    QDateTime tomorrow = now.addDays(1); // Tomorrow.
    tomorrow.setTime(QTime());           // Midnight.
    return now.msecsTo(tomorrow);
}
} // namespace

FriendListWidget::FriendListWidget(const Core& core_, Widget* parent, Settings& settings_,
                                   Style& style_, IMessageBoxManager& messageBoxManager_,
                                   FriendList& friendList_, ConferenceList& conferenceList_,
                                   Profile& profile_, bool conferencesOnTop)
    : QWidget(parent)
    , core{core_}
    , settings{settings_}
    , style{style_}
    , messageBoxManager{messageBoxManager_}
    , friendList{friendList_}
    , conferenceList{conferenceList_}
    , profile{profile_}
{
    const int countContacts = core.getFriendList().size();
    manager = new FriendListManager(countContacts, this);
    manager->setConferencesOnTop(conferencesOnTop);
    connect(manager, &FriendListManager::itemsChanged, this, &FriendListWidget::itemsChanged);

    listLayout = new QVBoxLayout;
    setLayout(listLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    listLayout->setSpacing(0);
    listLayout->setContentsMargins(0, 0, 0, 0);

    mode = settings.getFriendSortingMode();

    dayTimer = new QTimer(this);
    dayTimer->setObjectName("dayTimer");
    dayTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(dayTimer, &QTimer::timeout, this, &FriendListWidget::dayTimeout);
    dayTimer->start(timeUntilTomorrow());

    setAcceptDrops(true);
}

FriendListWidget::~FriendListWidget()
{
    for (int i = 0; i < settings.getCircleCount(); ++i) {
        CircleWidget* circle = CircleWidget::getFromID(i);
        delete circle;
    }
}

void FriendListWidget::setMode(SortingMode mode_)
{
    if (mode == mode_)
        return;

    mode = mode_;
    settings.setFriendSortingMode(mode);

    manager->setSortRequired();
}

void FriendListWidget::sortByMode()
{
    if (mode == SortingMode::Name) {
        manager->sortByName();
        if (!manager->getPositionsChanged()) {
            return;
        }

        cleanMainLayout();

        for (int i = 0; i < settings.getCircleCount(); ++i) {
            addCircleWidget(i);
        }

        const QVector<std::shared_ptr<IFriendListItem>> itemsTmp = manager->getItems(); // Sorted items
        QVector<IFriendListItem*> friendItems; // Items that are not included in the circle
        int posByName = 0;                     // Needed for scroll contacts
        // Linking a friend with a circle and setting scroll position
        for (auto& i : itemsTmp) {
            if (i->isFriend() && i->getCircleId() >= 0) {
                CircleWidget* circleWgt = CircleWidget::getFromID(i->getCircleId());
                if (circleWgt != nullptr) {
                    // Place a friend in the circle and continue
                    FriendWidget* frndTmp = qobject_cast<FriendWidget*>((i.get())->getWidget());
                    circleWgt->addFriendWidget(frndTmp, frndTmp->getFriend()->getStatus());
                    continue;
                }
            }
            // Place the item without the circle in the vector and set the position
            i->setNameSortedPos(posByName++);
            friendItems.push_back(i.get());
        }

        // Add conferences and friends without circles
        for (auto& friendItem : friendItems) {
            listLayout->addWidget(friendItem->getWidget());
        }

        // TODO: Try to remove
        manager->applyFilter();

        if (!manager->needHideCircles()) {
            // Sorts circles alphabetically and adds them to the layout
            QVector<CircleWidget*> circles;
            for (int i = 0; i < settings.getCircleCount(); ++i) {
                circles.push_back(CircleWidget::getFromID(i));
            }

            std::sort(circles.begin(), circles.end(), [](CircleWidget* a, CircleWidget* b) {
                return a->getName().toUpper() < b->getName().toUpper();
            });

            for (auto circle : circles) {

                const QVector<std::shared_ptr<IFriendListItem>> itemsInCircle =
                    getItemsFromCircle(circle);
                for (const auto& j : itemsInCircle) {
                    j->setNameSortedPos(posByName++);
                }

                listLayout->addWidget(circle);
            }
        }
    } else if (mode == SortingMode::Activity) {

        manager->sortByActivity();
        if (!manager->getPositionsChanged()) {
            return;
        }
        cleanMainLayout();

        const QLocale ql(settings.getTranslation());
        const QDate today = QDate::currentDate();
#define COMMENT "Category for sorting friends by activity"
        // clang-format off
        const QMap<Time, QString> names {
            { Time::Today,     tr("Today",                      COMMENT) },
            { Time::Yesterday, tr("Yesterday",                  COMMENT) },
            { Time::ThisWeek,  tr("Last 7 days",                COMMENT) },
            { Time::ThisMonth, tr("This month",                 COMMENT) },
            { Time::LongAgo,   tr("Older than 6 months",        COMMENT) },
            { Time::Never,     tr("Never",                      COMMENT) },
            { Time::Month1Ago, ql.monthName(today.addMonths(-1).month()) },
            { Time::Month2Ago, ql.monthName(today.addMonths(-2).month()) },
            { Time::Month3Ago, ql.monthName(today.addMonths(-3).month()) },
            { Time::Month4Ago, ql.monthName(today.addMonths(-4).month()) },
            { Time::Month5Ago, ql.monthName(today.addMonths(-5).month()) },
        };
// clang-format on
#undef COMMENT

        const QVector<std::shared_ptr<IFriendListItem>> itemsTmp = manager->getItems();

        for (auto& i : itemsTmp) {
            listLayout->addWidget(i->getWidget());
        }

        activityLayout = new QVBoxLayout();
        const bool compact = settings.getCompactLayout();
        for (const Time t : names.keys()) {
            CategoryWidget* category = new CategoryWidget(compact, settings, style, this);
            category->setName(names[t]);
            activityLayout->addWidget(category);
        }

        // TODO: Try to remove
        manager->applyFilter();

        // Insert widgets to CategoryWidget
        for (auto& i : itemsTmp) {
            if (i->isFriend()) {
                const int timeIndex = static_cast<int>(getTimeBucket(i->getLastActivity()));
                QWidget* widget = activityLayout->itemAt(timeIndex)->widget();
                CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(widget);
                FriendWidget* frnd = qobject_cast<FriendWidget*>((i.get())->getWidget());
                if (!isVisible() || (isVisible() && frnd->isVisible())) {
                    categoryWidget->addFriendWidget(frnd, frnd->getFriend()->getStatus());
                }
            }
        }

        // Hide empty categories
        for (int i = 0; i < activityLayout->count(); ++i) {
            QWidget* widget = activityLayout->itemAt(i)->widget();
            CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(widget);
            categoryWidget->setVisible(categoryWidget->hasChatRooms());
        }

        listLayout->addLayout(activityLayout);
    }
}

/**
 * @brief Clears the listLayout by performing the creation and ownership inverse of sortByMode.
 */
void FriendListWidget::cleanMainLayout()
{
    manager->resetParents();

    QLayoutItem* itemForDel;
    while ((itemForDel = listLayout->takeAt(0)) != nullptr) {
        listLayout->removeWidget(itemForDel->widget());
        QWidget* wgt = itemForDel->widget();
        if (wgt != nullptr) {
            wgt->setParent(nullptr);
        } else if (itemForDel->layout() != nullptr) {
            QLayout* layout = itemForDel->layout();
            QLayoutItem* itemTmp;
            while ((itemTmp = layout->takeAt(0)) != nullptr) {
                wgt = itemTmp->widget();
                delete wgt;
                delete itemTmp;
            }
        }
        delete itemForDel;
    }
}

QWidget* FriendListWidget::getNextWidgetForName(IFriendListItem* currentPos, bool forward) const
{
    const int pos = currentPos->getNameSortedPos();
    int nextPos = forward ? pos + 1 : pos - 1;
    if (nextPos >= manager->getItems().size()) {
        nextPos = 0;
    } else if (nextPos < 0) {
        nextPos = manager->getItems().size() - 1;
    }

    for (int i = 0; i < manager->getItems().size(); ++i) {
        if (manager->getItems().at(i)->getNameSortedPos() == nextPos) {
            return manager->getItems().at(i)->getWidget();
        }
    }
    return nullptr;
}

QVector<std::shared_ptr<IFriendListItem>> FriendListWidget::getItemsFromCircle(CircleWidget* circle) const
{
    const QVector<std::shared_ptr<IFriendListItem>> itemsTmp = manager->getItems();
    QVector<std::shared_ptr<IFriendListItem>> itemsInCircle;
    for (const auto& i : itemsTmp) {
        const int circleId = i->getCircleId();
        if (CircleWidget::getFromID(circleId) == circle) {
            itemsInCircle.push_back(i);
        }
    }
    return itemsInCircle;
}

CategoryWidget* FriendListWidget::getTimeCategoryWidget(const Friend* frd) const
{
    const auto activityTime = getActiveTimeFriend(frd, settings);
    const int timeIndex = static_cast<int>(getTimeBucket(activityTime));
    QWidget* widget = activityLayout->itemAt(timeIndex)->widget();
    return qobject_cast<CategoryWidget*>(widget);
}

FriendListWidget::SortingMode FriendListWidget::getMode() const
{
    return mode;
}

void FriendListWidget::addConferenceWidget(ConferenceWidget* widget)
{
    Conference* c = widget->getConference();
    connect(c, &Conference::titleChanged, this,
            [this, widget](const QString& author, const QString& name) {
                std::ignore = author;
                renameConferenceWidget(widget, name);
            });

    manager->addFriendListItem(widget);
}

void FriendListWidget::addFriendWidget(FriendWidget* w)
{
    manager->addFriendListItem(w);
}

void FriendListWidget::removeConferenceWidget(ConferenceWidget* w)
{
    manager->removeFriendListItem(w);
}

void FriendListWidget::removeFriendWidget(FriendWidget* w)
{
    const Friend* contact = w->getFriend();
    const int id = settings.getFriendCircleID(contact->getPublicKey());
    CircleWidget* circleWidget = CircleWidget::getFromID(id);
    if (circleWidget != nullptr) {
        circleWidget->removeFriendWidget(w, contact->getStatus());
        emit searchCircle(*circleWidget);
    }

    manager->removeFriendListItem(w);
}

void FriendListWidget::addCircleWidget(int id)
{
    createCircleWidget(id);
}

void FriendListWidget::addCircleWidget(FriendWidget* friendWidget)
{
    CircleWidget* circleWidget = createCircleWidget();
    if (circleWidget != nullptr) {
        if (friendWidget != nullptr) {
            const Friend* f = friendWidget->getFriend();
            circleWidget->addFriendWidget(friendWidget, f->getStatus());
            circleWidget->setExpanded(true);
        }

        if (window()->isActiveWindow())
            circleWidget->editName();

        manager->setSortRequired();
    }
}

void FriendListWidget::removeCircleWidget(CircleWidget* widget)
{
    widget->deleteLater();
}

void FriendListWidget::searchChatRooms(const QString& searchString, bool hideOnline,
                                       bool hideOffline, bool hideConferences)
{
    manager->setFilter(searchString, hideOnline, hideOffline, hideConferences);
}

void FriendListWidget::renameConferenceWidget(ConferenceWidget* conferenceWidget, const QString& newName)
{
    std::ignore = conferenceWidget;
    std::ignore = newName;
    itemsChanged();
}

void FriendListWidget::renameCircleWidget(CircleWidget* circleWidget, const QString& newName)
{
    circleWidget->setName(newName);

    if (mode == SortingMode::Name) {
        manager->setSortRequired();
    }
}

void FriendListWidget::onConferencePositionChanged(bool top)
{
    manager->setConferencesOnTop(top);

    if (mode != SortingMode::Name)
        return;

    itemsChanged();
}

void FriendListWidget::cycleChats(GenericChatroomWidget* activeChatroomWidget, bool forward)
{
    if (activeChatroomWidget == nullptr) {
        return;
    }

    int index = -1;
    FriendWidget* friendWidget = qobject_cast<FriendWidget*>(activeChatroomWidget);

    if (mode == SortingMode::Activity) {
        if (friendWidget == nullptr) {
            return;
        }

        const auto activityTime = getActiveTimeFriend(friendWidget->getFriend(), settings);
        index = static_cast<int>(getTimeBucket(activityTime));
        QWidget* widget_ = activityLayout->itemAt(index)->widget();
        CategoryWidget* categoryWidget = qobject_cast<CategoryWidget*>(widget_);

        if (categoryWidget == nullptr || categoryWidget->cycleChats(friendWidget, forward)) {
            return;
        }

        index += forward ? 1 : -1;

        for (;;) {
            // Bounds checking.
            if (index < 0) {
                index = LAST_TIME;
                continue;
            }
            if (index > LAST_TIME) {
                index = 0;
                continue;
            }

            auto* widget = activityLayout->itemAt(index)->widget();
            categoryWidget = qobject_cast<CategoryWidget*>(widget);

            if (categoryWidget != nullptr) {
                if (!categoryWidget->cycleChats(forward)) {
                    // Skip empty or finished categories.
                    index += forward ? 1 : -1;
                    continue;
                }
            }

            break;
        }

        return;
    }

    QWidget* wgt = nullptr;

    if (friendWidget != nullptr) {
        wgt = getNextWidgetForName(friendWidget, forward);
    } else {
        ConferenceWidget* conferenceWidget = qobject_cast<ConferenceWidget*>(activeChatroomWidget);
        wgt = getNextWidgetForName(conferenceWidget, forward);
    }

    FriendWidget* friendTmp = qobject_cast<FriendWidget*>(wgt);
    if (friendTmp != nullptr) {
        CircleWidget* circleWidget = CircleWidget::getFromID(friendTmp->getCircleId());
        if (circleWidget != nullptr) {
            circleWidget->setExpanded(true);
        }
    }

    GenericChatroomWidget* chatWidget = qobject_cast<GenericChatroomWidget*>(wgt);
    if (chatWidget != nullptr)
        emit chatWidget->chatroomWidgetClicked(chatWidget);
}

void FriendListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk toxPk(event->mimeData()->data("toxPk"));
    Friend* frnd = friendList.findFriend(toxPk);
    if (frnd != nullptr)
        event->acceptProposedAction();
}

void FriendListWidget::dropEvent(QDropEvent* event)
{
    // Check, that the element is dropped from qTox
    QObject* o = event->source();
    FriendWidget* widget = qobject_cast<FriendWidget*>(o);
    if (widget == nullptr)
        return;

    // Check, that the user has a friend with the same ToxPk
    assert(event->mimeData()->hasFormat("toxPk"));
    const ToxPk toxPk{event->mimeData()->data("toxPk")};
    Friend* f = friendList.findFriend(toxPk);
    if (f == nullptr)
        return;

    // Save CircleWidget before changing the Id
    const int circleId = settings.getFriendCircleID(f->getPublicKey());
    CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

    moveWidget(widget, f->getStatus(), true);

    if (circleWidget != nullptr)
        circleWidget->updateStatus();
}

void FriendListWidget::dayTimeout()
{
    if (mode == SortingMode::Activity) {
        itemsChanged();
    }

    dayTimer->start(timeUntilTomorrow());
}

void FriendListWidget::itemsChanged()
{
    sortByMode();
}

void FriendListWidget::moveWidget(FriendWidget* widget, Status::Status s, bool add)
{
    if (mode == SortingMode::Name) {
        const Friend* f = widget->getFriend();
        const int circleId = settings.getFriendCircleID(f->getPublicKey());
        CircleWidget* circleWidget = CircleWidget::getFromID(circleId);

        if (circleWidget == nullptr || add) {
            if (circleId != -1) {
                settings.setFriendCircleID(f->getPublicKey(), -1);
                manager->setSortRequired();
            } else {
                itemsChanged();
            }
            return;
        }

        circleWidget->addFriendWidget(widget, s);
    } else {
        const Friend* contact = widget->getFriend();
        auto* categoryWidget = getTimeCategoryWidget(contact);
        categoryWidget->addFriendWidget(widget, contact->getStatus());
        categoryWidget->show();
    }
    itemsChanged();
}

void FriendListWidget::updateActivityTime(const QDateTime& time)
{
    if (mode != SortingMode::Activity)
        return;

    const int timeIndex = static_cast<int>(getTimeBucket(time));
    QWidget* widget = activityLayout->itemAt(timeIndex)->widget();
    CategoryWidget* categoryWidget = static_cast<CategoryWidget*>(widget);
    categoryWidget->updateStatus();

    categoryWidget->setVisible(categoryWidget->hasChatRooms());
}

CircleWidget* FriendListWidget::createCircleWidget(int id)
{
    if (id == -1)
        id = settings.addCircle();

    if (CircleWidget::getFromID(id) != nullptr) {
        return CircleWidget::getFromID(id);
    }

    CircleWidget* circleWidget = new CircleWidget(core, this, id, settings, style, messageBoxManager,
                                                  friendList, conferenceList, profile);
    emit connectCircleWidget(*circleWidget);
    connect(this, &FriendListWidget::onCompactChanged, circleWidget, &CircleWidget::onCompactChanged);
    connect(circleWidget, &CircleWidget::renameRequested, this, &FriendListWidget::renameCircleWidget);

    return circleWidget;
}
