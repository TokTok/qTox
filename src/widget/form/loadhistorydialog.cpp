/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "loadhistorydialog.h"
#include "ui_loadhistorydialog.h"
#include "src/model/ichatlog.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/profile.h"
#include <QCalendarWidget>
#include <QDate>
#include <QTextCharFormat>

LoadHistoryDialog::LoadHistoryDialog(const IChatLog* chatLog_, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoadHistoryDialog)
    , chatLog(chatLog_)
{
    ui->setupUi(this);
    highlightDates(QDate::currentDate().year(), QDate::currentDate().month());
    connect(ui->fromDate, &QCalendarWidget::currentPageChanged, this,
            &LoadHistoryDialog::highlightDates);
}

LoadHistoryDialog::LoadHistoryDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoadHistoryDialog)
{
    ui->setupUi(this);
}

LoadHistoryDialog::~LoadHistoryDialog()
{
    delete ui;
}

QDateTime LoadHistoryDialog::getFromDate()
{
    QDateTime res(ui->fromDate->selectedDate().startOfDay());
    if (res.date().month() != ui->fromDate->monthShown()
        || res.date().year() != ui->fromDate->yearShown()) {
        const QDate newDate(ui->fromDate->yearShown(), ui->fromDate->monthShown(), 1);
        res.setDate(newDate);
    }

    return res;
}

void LoadHistoryDialog::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void LoadHistoryDialog::setInfoLabel(const QString& info)
{
    ui->fromLabel->setText(info);
}

void LoadHistoryDialog::highlightDates(int year, int month)
{
    const QDate monthStart(year, month, 1);
    const QDate monthEnd(year, month + 1, 1);

    // Max 31 days in a month
    auto dateIdxs = chatLog->getDateIdxs(monthStart, 31);

    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);

    QCalendarWidget* calendar = ui->fromDate;
    for (const auto& item : dateIdxs) {
        if (item.date < monthEnd) {
            calendar->setDateTextFormat(item.date, format);
        }
    }
}
