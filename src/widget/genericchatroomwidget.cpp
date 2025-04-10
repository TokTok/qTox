/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "genericchatroomwidget.h"

#include "maskablepixmapwidget.h"

#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"

#include <QBoxLayout>
#include <QMouseEvent>

GenericChatroomWidget::GenericChatroomWidget(bool compact_, Settings& settings_, Style& style_,
                                             QWidget* parent)
    : GenericChatItemWidget(compact_, style_, parent)
    , active{false}
    , settings{settings_}
    , style{style_}
{
    // avatar
    QSize size;
    if (isCompact())
        size = QSize(20, 20);
    else
        size = QSize(40, 40);

    avatar = new MaskablePixmapWidget(this, size, ":/img/avatar_mask.svg");

    // status text
    statusMessageLabel = new CroppingLabel(this);
    statusMessageLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setForegroundRole(QPalette::WindowText);
    statusMessageLabel->setObjectName("statusMessageLabelObj");

    nameLabel->setForegroundRole(QPalette::WindowText);
    nameLabel->setObjectName("nameLabelObj");

    connect(&settings, &Settings::compactLayoutChanged, this, &GenericChatroomWidget::compactChange);

    setAutoFillBackground(true);
    reloadTheme();

    compactChange(isCompact());
}

bool GenericChatroomWidget::eventFilter(QObject* object, QEvent* event)
{
    std::ignore = object;
    std::ignore = event;
    return true; // Disable all events.
}

void GenericChatroomWidget::compactChange(bool _compact)
{
    if (!isCompact())
        delete textLayout; // has to be first, deleted by layout

    setCompact(_compact);

    delete mainLayout;

    mainLayout = new QHBoxLayout;
    textLayout = new QVBoxLayout;

    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(0);
    textLayout->setContentsMargins(0, 0, 0, 0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    // avatar
    if (isCompact()) {
        delete textLayout; // Not needed
        setFixedHeight(25);
        avatar->setSize(QSize(20, 20));
        mainLayout->addSpacing(18);
        mainLayout->addWidget(avatar);
        mainLayout->addSpacing(5);
        mainLayout->addWidget(nameLabel);
        mainLayout->addWidget(statusMessageLabel);
        mainLayout->addSpacing(5);
        mainLayout->addWidget(&statusPic);
        mainLayout->addSpacing(5);
        mainLayout->activate();
        statusMessageLabel->setFont(Style::getFont(Style::Font::Small));
        nameLabel->setFont(Style::getFont(Style::Font::Medium));
    } else {
        setFixedHeight(55);
        avatar->setSize(QSize(40, 40));
        textLayout->addStretch();
        textLayout->addWidget(nameLabel);
        textLayout->addWidget(statusMessageLabel);
        textLayout->addStretch();
        mainLayout->addSpacing(20);
        mainLayout->addWidget(avatar);
        mainLayout->addSpacing(10);
        mainLayout->addLayout(textLayout);
        mainLayout->addSpacing(10);
        mainLayout->addWidget(&statusPic);
        mainLayout->addSpacing(10);
        mainLayout->activate();
        statusMessageLabel->setFont(Style::getFont(Style::Font::Medium));
        nameLabel->setFont(Style::getFont(Style::Font::Big));
    }
}

bool GenericChatroomWidget::isActive() const
{
    return active;
}

void GenericChatroomWidget::setActive(bool _active)
{
    active = _active;

    setProperty("active", active);
    nameLabel->setProperty("active", active);
    statusMessageLabel->setProperty("active", active);
    Style::repolish(this);
}

void GenericChatroomWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GenericChatroomWidget::setStatusMsg(const QString& status)
{
    statusMessageLabel->setText(status);
}

QString GenericChatroomWidget::getStatusMsg() const
{
    return statusMessageLabel->text();
}

QString GenericChatroomWidget::getTitle() const
{
    QString title = getName();

    if (!getStatusString().isNull())
        title += QStringLiteral(" (") + getStatusString() + QStringLiteral(")");

    return title;
}

void GenericChatroomWidget::reloadTheme()
{
    setStyleSheet(style.getStylesheet("genericChatRoomWidget/genericChatRoomWidget.qss", settings));
}

void GenericChatroomWidget::activate()
{
    emit chatroomWidgetClicked(this);
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit chatroomWidgetClicked(this);
    } else if (event->button() == Qt::MiddleButton) {
        emit middleMouseClicked();
    } else {
        event->ignore();
    }
}

void GenericChatroomWidget::enterEvent(QEnterEvent* event)
{
    std::ignore = event;
    if (!active)
        setBackgroundRole(QPalette::Highlight);
}

void GenericChatroomWidget::leaveEvent(QEvent* event)
{
    if (!active)
        setBackgroundRole(QPalette::Window);
    QWidget::leaveEvent(event);
}
