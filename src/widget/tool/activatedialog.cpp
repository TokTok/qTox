/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */


#include "activatedialog.h"
#include "src/widget/style.h"
#include <QEvent>

ActivateDialog::ActivateDialog(Style& style, QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    connect(&style, &Style::themeReload, this, &ActivateDialog::reloadTheme);
}

bool ActivateDialog::event(QEvent* event)
{
    if (event->type() == QEvent::WindowActivate || event->type() == QEvent::WindowStateChange)
        emit windowStateChanged(windowState());

    return QDialog::event(event);
}
