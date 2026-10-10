/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/widget/form/settings/genericsettings.h"

#include <memory>

class Paths;
class QTimer;
class Style;

namespace Ui {
class DebugNetProf;
}

class DebugNetProfForm final : public GenericForm
{
    Q_OBJECT
public:
    DebugNetProfForm(Style& style, QWidget* parent);
    ~DebugNetProfForm() override;
    QString getFormName() final
    {
        return tr("Network Profiler");
    }

private:
    void retranslateUi();

private:
    std::unique_ptr<Ui::DebugNetProf> ui_;
    QTimer* reloadTimer_;
};
