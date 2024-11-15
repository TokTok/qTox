/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/toxpk.h"
#include <QDateTime>
#include <QDialog>

namespace Ui {
class LoadHistoryDialog;
}
class IChatLog;

class LoadHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadHistoryDialog(const IChatLog* chatLog_, QWidget* parent = nullptr);
    explicit LoadHistoryDialog(QWidget* parent = nullptr);
    ~LoadHistoryDialog();

    QDateTime getFromDate();
    void setTitle(const QString& title);
    void setInfoLabel(const QString& info);

public slots:
    void highlightDates(int year, int month);

private:
    Ui::LoadHistoryDialog* ui;
    const IChatLog* chatLog;
};
