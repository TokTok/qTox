/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>
#include "src/widget/searchtypes.h"

namespace Ui {
class SearchSettingsForm;
}
class Settings;
class Style;

class SearchSettingsForm : public QWidget
{
    Q_OBJECT

public:
    SearchSettingsForm(Settings& settings, Style& style, QWidget *parent = nullptr);
    ~SearchSettingsForm();

    ParameterSearch getParameterSearch();
    void reloadTheme();

private:
    Ui::SearchSettingsForm *ui;
    QDate startDate;
    bool isUpdate{false};
    Settings& settings;
    Style& style;

    void updateStartDateLabel();
    void setUpdate(const bool isUpdate_);

private slots:
    void onStartSearchSelected(const int index);
    void onRegisterClicked(const bool checked);
    void onWordsOnlyClicked(const bool checked);
    void onRegularClicked(const bool checked);
    void onChoiceDate();

signals:
    void updateSettings(const bool isUpdate);
};
