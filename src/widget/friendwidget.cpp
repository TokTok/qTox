/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "friendwidget.h"

#include "circlewidget.h"
#include "conferencewidget.h"
#include "friendlistwidget.h"
#include "maskablepixmapwidget.h"

#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/about/aboutfriend.h"
#include "src/model/chatroom/friendchatroom.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/persistence/settings.h"
#include "src/widget/about/aboutfriendform.h"
#include "src/widget/form/chatform.h"
#include "src/widget/popup.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/widget.h"

#include <QApplication>
#include <QBitmap>
#include <QContextMenuEvent>
#include <QDrag>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMimeData>

#include <utility>

/**
 * @class FriendWidget
 *
 * Widget, which displays brief information about friend.
 * For example, used on friend list.
 * When you click should open the chat with friend. Widget has a context menu.
 */
FriendWidget::FriendWidget(std::shared_ptr<FriendChatroom> chatroom_, bool compact_,
                           Settings& settings_, Style& style_,
                           IMessageBoxManager& messageBoxManager_, Profile& profile_, QWidget* parent)
    : GenericChatroomWidget(compact_, settings_, style_, parent)
    , chatroom{std::move(chatroom_)}
    , isDefaultAvatar{true}
    , settings{settings_}
    , style{style_}
    , messageBoxManager{messageBoxManager_}
    , profile{profile_}
{
    avatar->setPixmap(QPixmap(":/img/contact.svg"));
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Offline)));
    statusPic.setMargin(3);

    auto* frnd = chatroom->getFriend();
    nameLabel->setText(frnd->getDisplayedName());
    // update alias when edited
    connect(nameLabel, &CroppingLabel::editFinished, frnd, &Friend::setAlias);
    // update on changes of the displayed name
    connect(frnd, &Friend::displayedNameChanged, nameLabel, &CroppingLabel::setText);
    connect(chatroom.get(), &FriendChatroom::activeChanged, this, &FriendWidget::setActive);
    statusMessageLabel->setTextFormat(Qt::PlainText);
}

/**
 * @brief FriendWidget::contextMenuEvent
 * @param event Describe a context menu event
 *
 * Default context menu event handler.
 * Redirect all event information to the signal.
 */
void FriendWidget::contextMenuEvent(QContextMenuEvent* event)
{
    emit contextMenuCalled(event);
}

/**
 * @brief FriendWidget::onContextMenuCalled
 * @param event Redirected from native contextMenuEvent
 *
 * Context menu handler. Always should be called to FriendWidget from FriendList
 */
void FriendWidget::onContextMenuCalled(QContextMenuEvent* event)
{
    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }

    installEventFilter(this); // Disable leave event.

    QMenu menu;

    if (chatroom->possibleToOpenInNewWindow()) {
        auto* const openChatWindow = menu.addAction(tr("Open chat in new window"));
        connect(openChatWindow, &QAction::triggered, this, [this]() { emit newWindowOpened(this); });
    }

    if (chatroom->canBeRemovedFromWindow()) {
        auto* const removeChatWindow = menu.addAction(tr("Remove chat from this window"));
        connect(removeChatWindow, &QAction::triggered, this, &FriendWidget::removeChatWindow);
    }

    menu.addSeparator();
    QMenu* inviteMenu =
        menu.addMenu(tr("Invite to conference", "Menu to invite a friend to a conference"));
    inviteMenu->setEnabled(chatroom->canBeInvited());
    auto* const newConferenceAction = inviteMenu->addAction(tr("To new conference"));
    connect(newConferenceAction, &QAction::triggered, chatroom.get(),
            &FriendChatroom::inviteToNewConference);
    inviteMenu->addSeparator();

    for (const auto& conference : chatroom->getConferences()) {
        auto* const conferenceAction =
            inviteMenu->addAction(tr("Invite to conference '%1'").arg(conference.name));
        connect(conferenceAction, &QAction::triggered, this,
                [this, conference] { chatroom->inviteFriend(conference.conference); });
    }

    const auto circleId = chatroom->getCircleId();
    auto* circleMenu =
        menu.addMenu(tr("Move to circle...", "Menu to move a friend into a different circle"));

    auto* const newCircleAction = circleMenu->addAction(tr("To new circle"));
    connect(newCircleAction, &QAction::triggered, this, &FriendWidget::moveToNewCircle);

    if (circleId != -1) {
        const auto circleName = chatroom->getCircleName();
        auto* const removeCircleAction =
            circleMenu->addAction(tr("Remove from circle '%1'").arg(circleName));
        connect(removeCircleAction, &QAction::triggered, this, &FriendWidget::removeFromCircle);
    }

    circleMenu->addSeparator();

    for (const auto& circle : chatroom->getOtherCircles()) {
        auto* action = new QAction(tr("Move to circle \"%1\"").arg(circle.name), circleMenu);
        connect(action, &QAction::triggered, this, [this, circle] { moveToCircle(circle.circleId); });
        circleMenu->addAction(action);
    }

    auto* const setAlias = menu.addAction(tr("Set alias..."));
    connect(setAlias, &QAction::triggered, nameLabel, &CroppingLabel::editBegin);

    menu.addSeparator();
    auto* autoAccept =
        menu.addAction(tr("Auto accept files from this friend", "context menu entry"));
    autoAccept->setCheckable(true);
    autoAccept->setChecked(!chatroom->autoAcceptEnabled());
    connect(autoAccept, &QAction::triggered, this, &FriendWidget::changeAutoAccept);
    menu.addSeparator();

    if (chatroom->friendCanBeRemoved()) {
        const auto friendPk = chatroom->getFriend()->getPublicKey();
        auto* const removeAction =
            menu.addAction(tr("Remove friend", "Menu to remove the friend from the friend list"));
        connect(
            removeAction, &QAction::triggered, this,
            [this, friendPk] { emit removeFriend(friendPk); }, Qt::QueuedConnection);
    }

    menu.addSeparator();
    auto* const aboutWindow = menu.addAction(tr("Show details"));
    connect(aboutWindow, &QAction::triggered, this, &FriendWidget::showDetails);

    const auto pos = event->globalPos();
    menu.exec(pos);

    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void FriendWidget::removeChatWindow()
{
    chatroom->removeFriendFromDialogs();
}

namespace {

std::tuple<CircleWidget*, FriendListWidget*>
getCircleAndFriendList(const Friend* frnd, FriendWidget* fw, Settings& settings)
{
    const auto pk = frnd->getPublicKey();
    const auto circleId = settings.getFriendCircleID(pk);
    auto* circleWidget = CircleWidget::getFromID(circleId);
    auto* w =
        circleWidget != nullptr ? static_cast<QWidget*>(circleWidget) : static_cast<QWidget*>(fw);
    auto* friendList = qobject_cast<FriendListWidget*>(w->parentWidget());
    return std::make_tuple(circleWidget, friendList);
}

} // namespace

void FriendWidget::moveToNewCircle()
{
    auto* const frnd = chatroom->getFriend();
    CircleWidget* circleWidget;
    FriendListWidget* friendList;
    std::tie(circleWidget, friendList) = getCircleAndFriendList(frnd, this, settings);

    if (circleWidget != nullptr) {
        circleWidget->updateStatus();
    }

    if (friendList != nullptr) {
        friendList->addCircleWidget(this);
    } else {
        const auto pk = frnd->getPublicKey();
        auto& s = settings;
        auto circleId = s.addCircle();
        s.setFriendCircleID(pk, circleId);
    }
}

void FriendWidget::removeFromCircle()
{
    auto* const frnd = chatroom->getFriend();
    CircleWidget* circleWidget;
    FriendListWidget* friendList;
    std::tie(circleWidget, friendList) = getCircleAndFriendList(frnd, this, settings);

    if (friendList != nullptr) {
        friendList->moveWidget(this, frnd->getStatus(), true);
    } else {
        const auto pk = frnd->getPublicKey();
        auto& s = settings;
        s.setFriendCircleID(pk, -1);
    }

    if (circleWidget != nullptr) {
        circleWidget->updateStatus();
    }
}

void FriendWidget::moveToCircle(int newCircleId)
{
    auto* const frnd = chatroom->getFriend();
    const auto pk = frnd->getPublicKey();
    const auto oldCircleId = settings.getFriendCircleID(pk);
    auto& s = settings;
    auto* oldCircleWidget = CircleWidget::getFromID(oldCircleId);
    auto* newCircleWidget = CircleWidget::getFromID(newCircleId);

    if (newCircleWidget != nullptr) {
        newCircleWidget->addFriendWidget(this, frnd->getStatus());
        newCircleWidget->setExpanded(true);
        s.savePersonal();
    } else {
        s.setFriendCircleID(pk, newCircleId);
    }

    if (oldCircleWidget != nullptr) {
        oldCircleWidget->updateStatus();
    }
}

void FriendWidget::changeAutoAccept(bool enable)
{
    if (enable) {
        const auto oldDir = chatroom->getAutoAcceptDir();
        const auto newDir = Popup::getAutoAcceptDir(this, oldDir);
        chatroom->setAutoAcceptDir(newDir);
    } else {
        chatroom->disableAutoAccept();
    }
}
void FriendWidget::showDetails()
{
    auto* const frnd = chatroom->getFriend();
    auto* const iAbout = new AboutFriend(frnd, &settings, profile);
    std::unique_ptr<IAboutFriend> about = std::unique_ptr<IAboutFriend>(iAbout);
    auto* const aboutUser =
        new AboutFriendForm(std::move(about), settings, style, messageBoxManager, this);
    connect(aboutUser, &AboutFriendForm::historyRemoved, this, &FriendWidget::friendHistoryRemoved);
    aboutUser->show();
}

void FriendWidget::setAsActiveChatroom()
{
    setActive(true);
}

void FriendWidget::setAsInactiveChatroom()
{
    setActive(false);
}

void FriendWidget::setActive(bool active_)
{
    GenericChatroomWidget::setActive(active_);
    if (isDefaultAvatar) {
        const auto uri =
            active ? QStringLiteral(":img/contact_dark.svg") : QStringLiteral(":img/contact.svg");
        avatar->setPixmap(QPixmap{uri});
    }
}

void FriendWidget::updateStatusLight()
{
    auto* const frnd = chatroom->getFriend();
    const bool event = frnd->getEventFlag();
    statusPic.setPixmap(QPixmap(Status::getIconPath(frnd->getStatus(), event)));

    if (event) {
        const Settings& s = settings;
        const uint32_t circleId = s.getFriendCircleID(frnd->getPublicKey());
        CircleWidget* circleWidget = CircleWidget::getFromID(circleId);
        if (circleWidget != nullptr) {
            circleWidget->setExpanded(true);
        }

        emit updateFriendActivity(*frnd);
    }

    statusPic.setMargin(event ? 1 : 3);
}

QString FriendWidget::getStatusString() const
{
    auto* const frnd = chatroom->getFriend();
    const int status = static_cast<int>(frnd->getStatus());
    const bool event = frnd->getEventFlag();

    static const QVector<QString> names = {
        tr("Online"), tr("Away"), tr("Busy"), tr("Offline"), tr("Blocked"),
    };

    return event ? tr("New message") : names.value(status);
}

const Friend* FriendWidget::getFriend() const
{
    return chatroom->getFriend();
}

const Chat* FriendWidget::getChat() const
{
    return getFriend();
}

bool FriendWidget::isFriend() const
{
    return true;
}

bool FriendWidget::isConference() const
{
    return false;
}

bool FriendWidget::isOnline() const
{
    const auto* const frnd = getFriend();
    return Status::isOnline(frnd->getStatus());
}

bool FriendWidget::widgetIsVisible() const
{
    return isVisible();
}

QString FriendWidget::getNameItem() const
{
    return nameLabel->fullText();
}

QDateTime FriendWidget::getLastActivity() const
{
    auto* const frnd = chatroom->getFriend();
    return settings.getFriendActivity(frnd->getPublicKey());
}

QWidget* FriendWidget::getWidget()
{
    return this;
}

void FriendWidget::setWidgetVisible(bool visible)
{
    setVisible(visible);
}

int FriendWidget::getCircleId() const
{
    return chatroom->getCircleId();
}

void FriendWidget::resetEventFlags()
{
    chatroom->resetEventFlags();
}

void FriendWidget::onAvatarSet(const ToxPk& friendPk, const QPixmap& pic)
{
    auto* const frnd = chatroom->getFriend();
    if (friendPk != frnd->getPublicKey()) {
        return;
    }

    isDefaultAvatar = false;
    avatar->setPixmap(pic);
}

void FriendWidget::onAvatarRemoved(const ToxPk& friendPk)
{
    auto* const frnd = chatroom->getFriend();
    if (friendPk != frnd->getPublicKey()) {
        return;
    }

    isDefaultAvatar = true;

    const QString path =
        QString(":/img/contact%1.svg").arg(isActive() ? QStringLiteral("_dark") : QString());
    avatar->setPixmap(QPixmap(path));
}

void FriendWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        dragStartPos = ev->pos();
    }

    GenericChatroomWidget::mousePressEvent(ev);
}

void FriendWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    const int distance = (dragStartPos - ev->pos()).manhattanLength();
    if (distance > QApplication::startDragDistance()) {
        auto* mdata = new QMimeData;
        const Friend* frnd = getFriend();
        mdata->setText(frnd->getDisplayedName());
        mdata->setData("toxPk", frnd->getPublicKey().getByteArray());

        auto* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}
