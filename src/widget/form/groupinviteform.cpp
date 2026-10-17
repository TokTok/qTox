/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "groupinviteform.h"

#include "src/core/core.h"
#include "src/core/groupid.h"
#include "src/model/groupinvite.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/groupinvitewidget.h"
#include "src/widget/translator.h"

#include <QGroupBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWindow>

#include <algorithm>

/**
 * @class GroupInviteForm
 *
 * @brief This form contains all group invites you received
 */

GroupInviteForm::GroupInviteForm(Settings& settings_, Core& core_)
    : headWidget(new QWidget(this))
    , headLabel(new QLabel(this))
    , createButton(new QPushButton(this))
    , joinButton(new QPushButton(this))
    , inviteBox(new QGroupBox(this))
    , scroll(new QScrollArea(this))
    , settings{settings_}
    , core{core_}
{
    auto* layout = new QVBoxLayout(this);
    connect(createButton, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        const QString groupName = QInputDialog::getText(
            this, tr("Create group"), tr("Enter a name for the group"), QLineEdit::Normal,
            QString(), &ok);
        if (ok) {
            if (!groupName.isEmpty()) {
                emit groupCreate(groupName);
            } else {
                QMessageBox::warning(this, tr("Create group"), tr("Group name cannot be empty."));
            }
        }
    });
    connect(joinButton, &QPushButton::clicked, this, [this]() {
        bool ok = false;
        const QString chatIdHex = QInputDialog::getText(
            this, tr("Join group by ID"), tr("Enter the group Chat ID (64 hex characters):"),
            QLineEdit::Normal, QString(), &ok);
        if (ok) {
            if (!chatIdHex.isEmpty()) {
                const QString clean = chatIdHex.trimmed();
                const QByteArray rawId = QByteArray::fromHex(clean.toLatin1());
                if (rawId.size() != TOX_GROUP_CHAT_ID_SIZE) {
                    QMessageBox::warning(this, tr("Join group by ID"),
                                         tr("Invalid group ID. Expected 64 hex characters."));
                    return;
                }
                core.joinGroup(GroupId(rawId));
            } else {
                QMessageBox::warning(this, tr("Join group by ID"), tr("Group ID cannot be empty."));
                return;
            }
        }
    });

    auto* innerWidget = new QWidget(scroll);
    innerWidget->setLayout(new QVBoxLayout());
    innerWidget->layout()->setAlignment(Qt::AlignTop);
    scroll->setWidget(innerWidget);
    scroll->setWidgetResizable(true);

    auto* inviteLayout = new QVBoxLayout(inviteBox);
    inviteLayout->addWidget(scroll);

    layout->addWidget(createButton);
    layout->addWidget(joinButton);
    layout->addWidget(inviteBox);

    QFont bold;
    bold.setBold(true);

    headLabel->setFont(bold);
    auto* headLayout = new QHBoxLayout(headWidget);
    headLayout->addWidget(headLabel);

    retranslateUi();
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

GroupInviteForm::~GroupInviteForm()
{
    Translator::unregister(this);
}

/**
 * @brief Detects that form is shown
 * @return True if form is visible
 */
bool GroupInviteForm::isShown() const
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
void GroupInviteForm::show(ContentLayout* contentLayout)
{
    contentLayout->mainContent->layout()->addWidget(this);
    contentLayout->mainHead->layout()->addWidget(headWidget);
    QWidget::show();
    headWidget->show();
}

/**
 * @brief Adds group invite
 * @param inviteInfo Object which contains info about group invitation
 * @return true if notification is needed, false otherwise
 */
bool GroupInviteForm::addGroupInvite(const GroupInvite& inviteInfo)
{
    // supress duplicate invite messages
    for (GroupInviteWidget* existing : invites) {
        const GroupInvite& existingInvite = existing->getInviteInfo();
        if (existingInvite.getFriendId() == inviteInfo.getFriendId()
            && existingInvite.getInviteData() == inviteInfo.getInviteData()) {
            return false;
        }
    }

    auto* widget = new GroupInviteWidget(this, inviteInfo, settings, core);
    scroll->widget()->layout()->addWidget(widget);
    invites.append(widget);
    connect(widget, &GroupInviteWidget::accepted, this,
            [this](const GroupInvite& inviteInfo_) {
                deleteInviteWidget(inviteInfo_);
                emit groupInviteAccepted(inviteInfo_);
            });

    connect(widget, &GroupInviteWidget::rejected, this,
            [this](const GroupInvite& inviteInfo_) { deleteInviteWidget(inviteInfo_); });
    if (isVisible()) {
        emit groupInvitesSeen();
        return false;
    }
    return true;
}

void GroupInviteForm::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    emit groupInvitesSeen();
}

/**
 * @brief Deletes accepted/declined group invite widget
 * @param inviteInfo Invite information of accepted/declined widget
 */
void GroupInviteForm::deleteInviteWidget(const GroupInvite& inviteInfo)
{
    auto deletingWidget =
        std::find_if(invites.begin(), invites.end(), [=](const GroupInviteWidget* widget) {
            return inviteInfo == widget->getInviteInfo();
        });
    (*deletingWidget)->deleteLater();
    scroll->widget()->layout()->removeWidget(*deletingWidget);
    invites.erase(deletingWidget);
}

void GroupInviteForm::retranslateUi()
{
    headLabel->setText(tr("Groups"));
    if (createButton != nullptr) {
        createButton->setText(tr("Create new group"));
    }
    if (joinButton != nullptr) {
        joinButton->setText(tr("Join group by ID"));
    }
    inviteBox->setTitle(tr("Group invites"));
    for (GroupInviteWidget* invite : invites) {
        invite->retranslateUi();
    }
}
