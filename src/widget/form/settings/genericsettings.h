/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>

class Style;

class GenericForm : public QWidget
{
    Q_OBJECT
public:
    GenericForm(QPixmap icon, Style& style);
    virtual ~GenericForm() {}

    virtual QString getFormName() = 0;
    QPixmap getFormIcon();

public slots:
    virtual void reloadTheme() {}

protected:
    bool eventFilter(QObject* o, QEvent* e) final;
    void eventsInit();

protected:
    QPixmap formIcon;
};
