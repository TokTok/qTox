/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "genericsettings.h"

namespace Ui {
class GeneralSettings;
}

class SettingsWidget;
class Settings;
class Style;

class GeneralForm final : public GenericForm
{
    Q_OBJECT
public:
    GeneralForm(Settings& settings, Style& style);
    ~GeneralForm() override;
    QString getFormName() final
    {
        return tr("General");
    }

    static const QStringList& getLocales();

signals:
    void updateIcons();

private slots:
    void on_transComboBox_currentIndexChanged(int index);
    void on_cbAutorun_stateChanged();
    void on_cbSpellChecking_stateChanged();
    void on_showSystemTray_stateChanged();
    void on_startInTray_stateChanged();
    void on_closeToTray_stateChanged();
    void on_lightTrayIcon_stateChanged();
    void on_autoAwaySpinBox_editingFinished();
    void on_minimizeToTray_stateChanged();
    void on_statusChanges_stateChanged();
    void on_conferenceJoinLeaveMessages_stateChanged();
    void on_autoacceptFiles_stateChanged();
    void on_maxAutoAcceptSizeMB_editingFinished();
    void on_autoSaveFilesDir_clicked();
    void on_checkUpdates_stateChanged();
    void on_markdownComboBox_currentIndexChanged(int index);

private:
    void retranslateUi();

private:
    const std::unique_ptr<Ui::GeneralSettings> bodyUI;
    Settings& settings;
    Style& style;
};
