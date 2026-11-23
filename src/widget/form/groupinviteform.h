/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QWidget>

class ContentLayout;
class GroupInvite;
class GroupInviteWidget;

class QGroupBox;
class QLabel;
class QPushButton;
class QScrollArea;
class Settings;
class Core;

namespace Ui {
class MainWindow;
}

class GroupInviteForm : public QWidget
{
    Q_OBJECT
public:
    GroupInviteForm(Settings& settings, Core& core);
    ~GroupInviteForm() override;

    void show(ContentLayout* contentLayout);
    bool addGroupInvite(const GroupInvite& inviteInfo);
    bool isShown() const;

signals:
    void groupCreate(const QString& groupName);
    void groupInviteAccepted(const GroupInvite& inviteInfo);
    void groupInvitesSeen();

protected:
    void showEvent(QShowEvent* event) final;

private:
    void retranslateUi();
    void deleteInviteWidget(const GroupInvite& inviteInfo);

private:
    QWidget* headWidget;
    QLabel* headLabel;
    QPushButton* createButton;
    QPushButton* joinButton;
    QGroupBox* inviteBox;
    QList<GroupInviteWidget*> invites;
    QScrollArea* scroll;
    Settings& settings;
    Core& core;
};
