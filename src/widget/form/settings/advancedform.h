/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "genericsettings.h"

class Core;
class Settings;
class Style;
class IMessageBoxManager;

namespace Ui {
class AdvancedSettings;
}

class AdvancedForm : public GenericForm
{
    Q_OBJECT
public:
    AdvancedForm(Settings& settings, Style& style, IMessageBoxManager& messageBoxManager);
    ~AdvancedForm() override;
    QString getFormName() final
    {
        return tr("Advanced");
    }

private slots:
    // Portable
    void on_cbMakeToxPortable_stateChanged();
    void on_resetButton_clicked();
    // Debug
    void on_btnCopyDebug_clicked();
    void on_btnExportLog_clicked();
    void on_cbEnableDebug_stateChanged();
    // Connection
    void on_cbEnableIPv6_stateChanged();
    void on_cbEnableUDP_stateChanged();
    void on_cbEnableLanDiscovery_stateChanged();
    void on_proxyAddr_editingFinished();
    void on_proxyPort_valueChanged(int port);
    void on_proxyType_currentIndexChanged(int index);
    // Experimental
    void on_cbExperimentalSandbox_stateChanged();

private:
    void retranslateUi();

private:
    Ui::AdvancedSettings* bodyUI;
    Settings& settings;
    IMessageBoxManager& messageBoxManager;
};
