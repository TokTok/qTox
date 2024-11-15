/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/model/about/iaboutfriend.h"

#include <QDialog>
#include <QPointer>

#include <memory>

namespace Ui {
class AboutFriendForm;
}

class Settings;
class Style;
class IMessageBoxManager;

class AboutFriendForm : public QDialog
{
    Q_OBJECT

public:
    AboutFriendForm(std::unique_ptr<IAboutFriend> about, Settings& settings,
        Style& style, IMessageBoxManager& messageBoxManager, QWidget* parent = nullptr);
    ~AboutFriendForm();

private:
    Ui::AboutFriendForm* ui;
    const std::unique_ptr<IAboutFriend> about;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;

signals:
    void histroyRemoved();

public slots:
    void reloadTheme();

private slots:
    void onAutoAcceptDirChanged(const QString& path);
    void onAcceptedClicked();
    void onAutoAcceptDirClicked();
    void onAutoAcceptCallClicked();
    void onAutoGroupInvite();
    void onSelectDirClicked();
    void onRemoveHistoryClicked();
};
