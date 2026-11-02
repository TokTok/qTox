/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "groupwidget.h"

#include "maskablepixmapwidget.h"

#include "src/model/group.h"
#include "src/model/status.h"
#include "src/widget/friendwidget.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"
#include "tool/croppinglabel.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMimeData>
#include <QPalette>

GroupWidget::GroupWidget(std::shared_ptr<GroupRoom> chatroom_, bool compact_, Settings& settings_,
                         Style& style_, QWidget* parent)
    : GenericChatroomWidget(compact_, settings_, style_, parent)
    , chatroom{std::move(chatroom_)}
    , groupId{chatroom->getGroup()->getPersistentId()}
{
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()));
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Online)));
    statusPic.setMargin(3);

    Group* g = chatroom->getGroup();
    nameLabel->setText(g->getDisplayedName());

    updateUserCount(g->getPeersCount());
    setAcceptDrops(true);

    connect(g, &Group::titleChanged, this, &GroupWidget::updateTitle);
    connect(g, &Group::numPeersChanged, this, &GroupWidget::updateUserCount);
    connect(nameLabel, &CroppingLabel::editFinished, g, &Group::setName);
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

GroupWidget::~GroupWidget()
{
    Translator::unregister(this);
}

void GroupWidget::updateTitle(const QString& author, const QString& newName)
{
    std::ignore = author;
    nameLabel->setText(newName);
}

void GroupWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }

    installEventFilter(this); // Disable leave event.

    QMenu menu;

    QAction* openChatWindow = nullptr;
    if (chatroom->possibleToOpenInNewWindow()) {
        openChatWindow = menu.addAction(tr("Open chat in new window"));
    }

    QAction* removeChatWindow = nullptr;
    if (chatroom->canBeRemovedFromWindow()) {
        removeChatWindow = menu.addAction(tr("Remove chat from this window"));
    }

    menu.addSeparator();

    QAction* setTitle = menu.addAction(tr("Set title..."));
    auto* quitGroup = menu.addAction(tr("Quit group", "Menu to quit a group"));
    // Deleting the widget from inside the menu handler would destroy the
    // stack-allocated QMenu while it is still a child, so defer the removal.
    connect(quitGroup, &QAction::triggered, this, [this]() { emit removeGroup(groupId); },
            Qt::QueuedConnection);

    QAction* selectedItem = menu.exec(event->globalPos());

    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }

    if (selectedItem == nullptr) {
        return;
    }

    if (selectedItem == openChatWindow) {
        emit newWindowOpened(this);
    } else if (selectedItem == removeChatWindow) {
        chatroom->removeGroupFromDialogs();
    } else if (selectedItem == setTitle) {
        editName();
    }
}

void GroupWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        dragStartPos = ev->pos();
    }

    GenericChatroomWidget::mousePressEvent(ev);
}

void GroupWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance()) {
        auto* mdata = new QMimeData;
        const Group* group = getGroup();
        mdata->setText(group->getDisplayedName());
        mdata->setData("groupId", group->getPersistentId().getByteArray());

        auto* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void GroupWidget::updateUserCount(int numPeers)
{
    statusMessageLabel->setText(tr("%n user(s) in chat", "Number of users in chat", numPeers));
}

void GroupWidget::setAsActiveChatroom()
{
    setActive(true);
    avatar->setPixmap(Style::scaleSvgImage(":img/group_dark.svg", avatar->width(), avatar->height()));
}

void GroupWidget::setAsInactiveChatroom()
{
    setActive(false);
    avatar->setPixmap(Style::scaleSvgImage(":img/group.svg", avatar->width(), avatar->height()));
}

/*
 * @brief GroupWidget::startCall light up the on call indicator.
 */
void GroupWidget::startCall()
{
    updateStatusLight();
}

/*
 * @brief GroupWidget::stopCall shut down the on call indicator.
 */
void GroupWidget::stopCall()
{
    updateStatusLight();
}

void GroupWidget::updateStatusLight()
{
    Group* g = chatroom->getGroup();

    const bool event = g->getEventFlag();
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Online, event)));
    statusPic.setMargin(event ? 1 : 3);
}

QString GroupWidget::getStatusString() const
{
    if (chatroom->hasNewMessage()) {
        return tr("New message");
    }
    return tr("Online");
}

void GroupWidget::editName()
{
    nameLabel->editBegin();
}

bool GroupWidget::isFriend() const
{
    return false;
}

bool GroupWidget::isConference() const
{
    return false;
}

bool GroupWidget::isGroup() const
{
    return true;
}

QString GroupWidget::getNameItem() const
{
    return nameLabel->fullText();
}

bool GroupWidget::isOnline() const
{
    return true;
}

bool GroupWidget::widgetIsVisible() const
{
    return isVisible();
}

QDateTime GroupWidget::getLastActivity() const
{
    return QDateTime::currentDateTime();
}

QWidget* GroupWidget::getWidget()
{
    return this;
}

void GroupWidget::setWidgetVisible(bool visible)
{
    setVisible(visible);
}

Group* GroupWidget::getGroup() const
{
    return chatroom->getGroup();
}

const Chat* GroupWidget::getChat() const
{
    return getGroup();
}

void GroupWidget::resetEventFlags()
{
    chatroom->resetEventFlags();
}

void GroupWidget::dragEnterEvent(QDragEnterEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk pk{ev->mimeData()->data("toxPk")};
    if (chatroom->friendExists(pk)) {
        ev->acceptProposedAction();
    }

    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }
}

void GroupWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    std::ignore = event;
    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void GroupWidget::dropEvent(QDropEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk pk{ev->mimeData()->data("toxPk")};
    if (!chatroom->friendExists(pk)) {
        return;
    }

    chatroom->inviteFriend(pk);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void GroupWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GroupWidget::retranslateUi()
{
    const Group* group = chatroom->getGroup();
    updateUserCount(group->getPeersCount());
}
