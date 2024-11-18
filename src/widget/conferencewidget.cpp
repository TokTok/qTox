/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "conferencewidget.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QMenu>
#include <QMimeData>
#include <QPalette>

#include "maskablepixmapwidget.h"
#include "form/conferenceform.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/conferencelist.h"
#include "src/model/friend.h"
#include "src/model/conference.h"
#include "src/model/status.h"
#include "src/widget/friendwidget.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"
#include "tool/croppinglabel.h"

ConferenceWidget::ConferenceWidget(std::shared_ptr<ConferenceRoom> chatroom_, bool compact_,
                         Settings& settings_, Style& style_)
    : GenericChatroomWidget(compact_, settings_, style_)
    , conferenceId{chatroom_->getConference()->getPersistentId()}
    , chatroom{chatroom_}
{
    avatar->setPixmap(Style::scaleSvgImage(":img/conference.svg", avatar->width(), avatar->height()));
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Online)));
    statusPic.setMargin(3);

    Conference* c = chatroom->getConference();
    nameLabel->setText(c->getName());

    updateUserCount(c->getPeersCount());
    setAcceptDrops(true);

    connect(c, &Conference::titleChanged, this, &ConferenceWidget::updateTitle);
    connect(c, &Conference::numPeersChanged, this, &ConferenceWidget::updateUserCount);
    connect(nameLabel, &CroppingLabel::editFinished, c, &Conference::setName);
    Translator::registerHandler(std::bind(&ConferenceWidget::retranslateUi, this), this);
}

ConferenceWidget::~ConferenceWidget()
{
    Translator::unregister(this);
}

void ConferenceWidget::updateTitle(const QString& author, const QString& newName)
{
    std::ignore = author;
    nameLabel->setText(newName);
}

void ConferenceWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!active) {
        setBackgroundRole(QPalette::Highlight);
    }

    installEventFilter(this); // Disable leave event.

    QMenu menu(this);

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
    QAction* quitConference = menu.addAction(tr("Quit conference", "Menu to quit a conference"));

    QAction* selectedItem = menu.exec(event->globalPos());

    removeEventFilter(this);

    if (!active) {
        setBackgroundRole(QPalette::Window);
    }

    if (!selectedItem) {
        return;
    }

    if (selectedItem == quitConference) {
        emit removeConference(conferenceId);
    } else if (selectedItem == openChatWindow) {
        emit newWindowOpened(this);
    } else if (selectedItem == removeChatWindow) {
        chatroom->removeConferenceFromDialogs();
    } else if (selectedItem == setTitle) {
        editName();
    }
}

void ConferenceWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        dragStartPos = ev->pos();
    }

    GenericChatroomWidget::mousePressEvent(ev);
}

void ConferenceWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (!(ev->buttons() & Qt::LeftButton)) {
        return;
    }

    if ((dragStartPos - ev->pos()).manhattanLength() > QApplication::startDragDistance()) {
        QMimeData* mdata = new QMimeData;
        const Conference* conference = getConference();
        mdata->setText(conference->getName());
        mdata->setData("conferenceId", conference->getPersistentId().getByteArray());

        QDrag* drag = new QDrag(this);
        drag->setMimeData(mdata);
        drag->setPixmap(avatar->getPixmap());
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
}

void ConferenceWidget::updateUserCount(int numPeers)
{
    statusMessageLabel->setText(tr("%n user(s) in chat", "Number of users in chat", numPeers));
}

void ConferenceWidget::setAsActiveChatroom()
{
    setActive(true);
    avatar->setPixmap(Style::scaleSvgImage(":img/conference_dark.svg", avatar->width(), avatar->height()));
}

void ConferenceWidget::setAsInactiveChatroom()
{
    setActive(false);
    avatar->setPixmap(Style::scaleSvgImage(":img/conference.svg", avatar->width(), avatar->height()));
}

void ConferenceWidget::updateStatusLight()
{
    Conference* c = chatroom->getConference();

    const bool event = c->getEventFlag();
    statusPic.setPixmap(QPixmap(Status::getIconPath(Status::Status::Online, event)));
    statusPic.setMargin(event ? 1 : 3);
}

QString ConferenceWidget::getStatusString() const
{
    if (chatroom->hasNewMessage()) {
        return tr("New Message");
    } else {
        return tr("Online");
    }
}

void ConferenceWidget::editName()
{
    nameLabel->editBegin();
}

bool ConferenceWidget::isFriend() const
{
    return false;
}

bool ConferenceWidget::isConference() const
{
    return true;
}

QString ConferenceWidget::getNameItem() const
{
    return nameLabel->fullText();
}

bool ConferenceWidget::isOnline() const
{
    return true;
}

bool ConferenceWidget::widgetIsVisible() const
{
    return isVisible();
}

QDateTime ConferenceWidget::getLastActivity() const
{
    return QDateTime::currentDateTime();
}

QWidget* ConferenceWidget::getWidget()
{
    return this;
}

void ConferenceWidget::setWidgetVisible(bool visible)
{
    setVisible(visible);
}

// TODO: Remove
Conference* ConferenceWidget::getConference() const
{
    return chatroom->getConference();
}

const Chat* ConferenceWidget::getChat() const
{
    return getConference();
}

void ConferenceWidget::resetEventFlags()
{
    chatroom->resetEventFlags();
}

void ConferenceWidget::dragEnterEvent(QDragEnterEvent* ev)
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

void ConferenceWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    std::ignore = event;
    if (!active) {
        setBackgroundRole(QPalette::Window);
    }
}

void ConferenceWidget::dropEvent(QDropEvent* ev)
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

void ConferenceWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void ConferenceWidget::retranslateUi()
{
    const Conference* conference = chatroom->getConference();
    updateUserCount(conference->getPeersCount());
}
