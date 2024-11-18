/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "conferenceinviteform.h"

#include "ui_mainwindow.h"
#include "src/core/core.h"
#include "src/model/conferenceinvite.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/conferenceinvitewidget.h"
#include "src/widget/translator.h"

#include <QDateTime>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QWindow>

#include <algorithm>
#include <tox/tox.h>

/**
 * @class ConferenceInviteForm
 *
 * @brief This form contains all conference invites you received
 */

ConferenceInviteForm::ConferenceInviteForm(Settings& settings_, Core& core_)
    : headWidget(new QWidget(this))
    , headLabel(new QLabel(this))
    , createButton(new QPushButton(this))
    , inviteBox(new QGroupBox(this))
    , scroll(new QScrollArea(this))
    , settings{settings_}
    , core{core_}
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    connect(createButton, &QPushButton::clicked, this,
            [this]() { emit conferenceCreate(TOX_CONFERENCE_TYPE_AV); });

    QWidget* innerWidget = new QWidget(scroll);
    innerWidget->setLayout(new QVBoxLayout());
    innerWidget->layout()->setAlignment(Qt::AlignTop);
    scroll->setWidget(innerWidget);
    scroll->setWidgetResizable(true);

    QVBoxLayout* inviteLayout = new QVBoxLayout(inviteBox);
    inviteLayout->addWidget(scroll);

    layout->addWidget(createButton);
    layout->addWidget(inviteBox);

    QFont bold;
    bold.setBold(true);

    headLabel->setFont(bold);
    QHBoxLayout* headLayout = new QHBoxLayout(headWidget);
    headLayout->addWidget(headLabel);

    retranslateUi();
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

ConferenceInviteForm::~ConferenceInviteForm()
{
    Translator::unregister(this);
}

/**
 * @brief Detects that form is shown
 * @return True if form is visible
 */
bool ConferenceInviteForm::isShown() const
{
    const bool result = isVisible();
    if (result) {
        headWidget->window()->windowHandle()->alert(0);
    }
    return result;
}

/**
 * @brief Shows the form
 * @param contentLayout Main layout that contains all components of the form
 */
void ConferenceInviteForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(this);
    contentLayout->mainHead->layout()->addWidget(headWidget);
    QWidget::show();
    headWidget->show();
}

/**
 * @brief Adds conference invite
 * @param inviteInfo Object which contains info about conference invitation
 * @return true if notification is needed, false otherwise
 */
bool ConferenceInviteForm::addConferenceInvite(const ConferenceInvite& inviteInfo)
{
    // supress duplicate invite messages
    for (ConferenceInviteWidget* existing : invites) {
        if (existing->getInviteInfo().getInvite() == inviteInfo.getInvite()) {
            return false;
        }
    }

    ConferenceInviteWidget* widget = new ConferenceInviteWidget(this, inviteInfo, settings, core);
    scroll->widget()->layout()->addWidget(widget);
    invites.append(widget);
    connect(widget, &ConferenceInviteWidget::accepted, this,
            [this](const ConferenceInvite& inviteInfo_) {
                deleteInviteWidget(inviteInfo_);
                emit conferenceInviteAccepted(inviteInfo_);
            });

    connect(widget, &ConferenceInviteWidget::rejected, this,
            [this](const ConferenceInvite& inviteInfo_) { deleteInviteWidget(inviteInfo_); });
    if (isVisible()) {
        emit conferenceInvitesSeen();
        return false;
    }
    return true;
}

void ConferenceInviteForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit conferenceInvitesSeen();
}

/**
 * @brief Deletes accepted/declined conference invite widget
 * @param inviteInfo Invite information of accepted/declined widget
 */
void ConferenceInviteForm::deleteInviteWidget(const ConferenceInvite& inviteInfo)
{
    auto deletingWidget =
        std::find_if(invites.begin(), invites.end(), [=](const ConferenceInviteWidget* widget) {
            return inviteInfo == widget->getInviteInfo();
        });
    (*deletingWidget)->deleteLater();
    scroll->widget()->layout()->removeWidget(*deletingWidget);
    invites.erase(deletingWidget);
}

void ConferenceInviteForm::retranslateUi()
{
    headLabel->setText(tr("Conferences"));
    if (createButton != nullptr) {
        createButton->setText(tr("Create new conference"));
    }
    inviteBox->setTitle(tr("Conference invites"));
    for (ConferenceInviteWidget* invite : invites) {
        invite->retranslateUi();
    }
}
