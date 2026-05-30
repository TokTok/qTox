/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/core/conferencetype.h"

#include <QWidget>

class ContentLayout;
class ConferenceInvite;
class ConferenceInviteWidget;

class QGroupBox;
class QLabel;
class QPushButton;
class QScrollArea;
class QSignalMapper;
class Settings;
class Core;

namespace Ui {
class MainWindow;
}

class ConferenceInviteForm : public QWidget
{
    Q_OBJECT
public:
    ConferenceInviteForm(Settings& settings, Core& core);
    ~ConferenceInviteForm() override;

    void show(ContentLayout* contentLayout);
    bool addConferenceInvite(const ConferenceInvite& inviteInfo);
    bool isShown() const;

signals:
    void conferenceCreate(ConferenceType type);
    void conferenceInviteAccepted(const ConferenceInvite& inviteInfo);
    void conferenceInvitesSeen();

protected:
    void showEvent(QShowEvent* event) final;

private:
    void retranslateUi();
    void deleteInviteWidget(const ConferenceInvite& inviteInfo);

private:
    QWidget* headWidget;
    QLabel* headLabel;
    QPushButton* createButton;
    QGroupBox* inviteBox;
    QList<ConferenceInviteWidget*> invites;
    QScrollArea* scroll;
    Settings& settings;
    Core& core;
};
