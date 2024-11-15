/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/groupinvite.h"

#include <QWidget>

class CroppingLabel;

class QHBoxLayout;
class QPushButton;
class Settings;
class Core;

class GroupInviteWidget : public QWidget
{
    Q_OBJECT
public:
    GroupInviteWidget(QWidget* parent, const GroupInvite& invite, Settings& settings,
        Core& core);
    void retranslateUi();
    const GroupInvite getInviteInfo() const;

signals:
    void accepted(const GroupInvite& invite);
    void rejected(const GroupInvite& invite);

private:
    QPushButton* acceptButton;
    QPushButton* rejectButton;
    CroppingLabel* inviteMessageLabel;
    QHBoxLayout* widgetLayout;
    GroupInvite inviteInfo;
    Settings& settings;
    Core& core;
};
