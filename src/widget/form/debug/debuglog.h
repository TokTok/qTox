/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/widget/form/settings/genericsettings.h"

#include <memory>

class DebugLogModel;
class Paths;
class QTimer;
class Style;

namespace Ui {
class DebugLog;
}

class DebugLogForm : public GenericForm
{
    Q_OBJECT
public:
    DebugLogForm(Paths& paths, Style& style);
    ~DebugLogForm();
    QString getFormName() final
    {
        return tr("Debug Log");
    }

private:
    void retranslateUi();

private:
    std::unique_ptr<Ui::DebugLog> ui_;
    std::unique_ptr<DebugLogModel> debugLogModel_;
    std::unique_ptr<QTimer> reloadTimer_;
};
