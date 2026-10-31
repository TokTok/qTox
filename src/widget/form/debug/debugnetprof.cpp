/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "debugnetprof.h"
#include "ui_debugnetprof.h"

#include "src/model/debug/debuglogmodel.h"
#include "src/persistence/paths.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"

#include <tox/tox.h>

#include <QTimer>

#include <memory>

DebugNetProfForm::DebugNetProfForm(Style& style, QWidget* parent)
    : GenericForm{QPixmap(":/img/settings/general.png"), style, parent}
    , ui_{std::make_unique<Ui::DebugNetProf>()}
    , reloadTimer_{new QTimer(this)}
{
    ui_->setupUi(this);

    // Reload logs every 5 seconds
    reloadTimer_->start(5000);

    Translator::registerHandler([this] { retranslateUi(); }, this);
}

DebugNetProfForm::~DebugNetProfForm()
{
    Translator::unregister(this);
}

/**
 * @brief Retranslate all elements in the form.
 */
void DebugNetProfForm::retranslateUi()
{
    ui_->retranslateUi(this);
}
