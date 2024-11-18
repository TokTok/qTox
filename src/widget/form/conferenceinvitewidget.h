/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/conferenceinvite.h"

#include <QWidget>

class CroppingLabel;

class QHBoxLayout;
class QPushButton;
class Settings;
class Core;

class ConferenceInviteWidget : public QWidget
{
    Q_OBJECT
public:
    ConferenceInviteWidget(QWidget* parent, const ConferenceInvite& invite, Settings& settings, Core& core);
    void retranslateUi();
    const ConferenceInvite getInviteInfo() const;

signals:
    void accepted(const ConferenceInvite& invite);
    void rejected(const ConferenceInvite& invite);

private:
    QPushButton* acceptButton;
    QPushButton* rejectButton;
    CroppingLabel* inviteMessageLabel;
    QHBoxLayout* widgetLayout;
    ConferenceInvite inviteInfo;
    Settings& settings;
    Core& core;
};
