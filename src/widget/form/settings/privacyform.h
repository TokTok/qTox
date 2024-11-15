/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericsettings.h"

class Core;
class Settings;
class Style;
class Profile;

namespace Ui {
class PrivacySettings;
}

class PrivacyForm : public GenericForm
{
    Q_OBJECT
public:
    PrivacyForm(Core* core_, Settings& settings, Style& style, Profile& profile);
    ~PrivacyForm();
    QString getFormName() final
    {
        return tr("Privacy");
    }

signals:
    void clearAllReceipts();

private slots:
    void on_cbKeepHistory_stateChanged();
    void on_cbTypingNotification_stateChanged();
    void on_nospamLineEdit_editingFinished();
    void on_randomNosapamButton_clicked();
    void on_nospamLineEdit_textChanged();
    void on_blackListTextEdit_textChanged();
    void showEvent(QShowEvent* event) final;

private:
    void retranslateUi();

private:
    Ui::PrivacySettings* bodyUI;
    Core* core;
    Settings& settings;
    Profile& profile;
};
