/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QDialog>

class Style;

class ActivateDialog : public QDialog
{
    Q_OBJECT
public:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    ActivateDialog(Style& style, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
#else
    ActivateDialog(Style& style, QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
#endif
    bool event(QEvent* event) override;

public slots:
    virtual void reloadTheme() {}

signals:
    void windowStateChanged(Qt::WindowStates state);
};
