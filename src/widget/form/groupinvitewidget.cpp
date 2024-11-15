/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "groupinvitewidget.h"

#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/croppinglabel.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSignalMapper>

/**
 * @class GroupInviteWidget
 *
 * @brief This class shows information about single group invite
 * and provides buttons to accept/reject it
 */

GroupInviteWidget::GroupInviteWidget(QWidget* parent, const GroupInvite& invite,
    Settings& settings_, Core& core_)
    : QWidget(parent)
    , acceptButton(new QPushButton(this))
    , rejectButton(new QPushButton(this))
    , inviteMessageLabel(new CroppingLabel(this))
    , widgetLayout(new QHBoxLayout(this))
    , inviteInfo(invite)
    , settings{settings_}
    , core{core_}
{
    connect(acceptButton, &QPushButton::clicked, [this] { emit accepted(inviteInfo); });
    connect(rejectButton, &QPushButton::clicked, [this] { emit rejected(inviteInfo); });
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
    QString name = core.getFriendUsername(inviteInfo.getFriendId());
    QDateTime inviteDate = inviteInfo.getInviteDate();
    QString date = inviteDate.toString(settings.getDateFormat());
    QString time = inviteDate.toString(settings.getTimestampFormat());

    inviteMessageLabel->setText(
        tr("Invited by %1 on %2 at %3.").arg("<b>%1</b>").arg(name.toHtmlEscaped(), date, time));
    acceptButton->setText(tr("Join"));
    rejectButton->setText(tr("Decline"));
}

/**
 * @brief Returns infomation about invitation - e.g., who and when sent
 * @return Invite information object
 */
const GroupInvite GroupInviteWidget::getInviteInfo() const
{
    return inviteInfo;
}
