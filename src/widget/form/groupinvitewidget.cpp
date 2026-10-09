/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "groupinvitewidget.h"

#include "src/core/core.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/croppinglabel.h"

#include <QHBoxLayout>
#include <QPushButton>

#include <utility>

/**
 * @class GroupInviteWidget
 *
 * @brief This class shows information about single group invite
 * and provides buttons to accept/reject it
 */

GroupInviteWidget::GroupInviteWidget(QWidget* parent, GroupInvite invite, Settings& settings_,
                                     Core& core_)
    : QWidget(parent)
    , acceptButton(new QPushButton(this))
    , rejectButton(new QPushButton(this))
    , inviteMessageLabel(new CroppingLabel(this))
    , widgetLayout(new QHBoxLayout(this))
    , inviteInfo(std::move(invite))
    , settings{settings_}
    , core{core_}
{
    connect(acceptButton, &QPushButton::clicked, this, [this] { emit accepted(inviteInfo); });
    connect(rejectButton, &QPushButton::clicked, this, [this] { emit rejected(inviteInfo); });
    widgetLayout->addWidget(inviteMessageLabel);
    widgetLayout->addWidget(acceptButton);
    widgetLayout->addWidget(rejectButton);
    setLayout(widgetLayout);
    retranslateUi();
}

/**
 * @brief Retranslate all elements in the form.
 */
void GroupInviteWidget::retranslateUi()
{
    const QString name = core.getFriendUsername(inviteInfo.getFriendId());
    const QDateTime inviteDate = inviteInfo.getInviteDate();
    const QString date = inviteDate.toString(settings.getDateFormat());
    const QString time = inviteDate.toString(settings.getTimestampFormat());

    inviteMessageLabel->setText(
        tr("Invited by %1 to %2 on %3 at %4.")
            .arg(QStringLiteral("<b>%1</b>").arg(name.toHtmlEscaped()),
                 inviteInfo.getGroupName().toHtmlEscaped(), date, time));
    acceptButton->setText(tr("Join"));
    rejectButton->setText(tr("Decline"));
}

/**
 * @brief Returns infomation about invitation - e.g., who and when sent
 * @return Invite information object
 */
GroupInvite GroupInviteWidget::getInviteInfo() const
{
    return inviteInfo;
}
