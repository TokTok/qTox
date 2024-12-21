/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "setpassworddialog.h"
#include "ui_setpassworddialog.h"

#include <QApplication>
#include <QPushButton>
#include <QRegularExpression>

const double SetPasswordDialog::reasonablePasswordLength = 8.;

SetPasswordDialog::SetPasswordDialog(QString body_, QString extraButton, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SetPasswordDialog)
    , body(body_ + "\n\n")
{
    ui->setupUi(this);

    connect(ui->passwordlineEdit, &PasswordEdit::textChanged, this, &SetPasswordDialog::onPasswordEdit);
    connect(ui->repasswordlineEdit, &PasswordEdit::textChanged, this,
            &SetPasswordDialog::onPasswordEdit);

    ui->body->setText(body_ + "\n\n");
    QPushButton* ok = ui->buttonBox->button(QDialogButtonBox::Ok);
    ok->setEnabled(false);
    ok->setText(QApplication::tr("Ok"));
    QPushButton* cancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
    cancel->setText(QApplication::tr("Cancel"));

    if (!extraButton.isEmpty()) {
        QPushButton* third = new QPushButton(extraButton);
        ui->buttonBox->addButton(third, QDialogButtonBox::YesRole);
        connect(third, &QPushButton::clicked, this, [&]() { done(Tertiary); });
    }
}

SetPasswordDialog::~SetPasswordDialog()
{
    delete ui;
}

void SetPasswordDialog::onPasswordEdit()
{
    QString password = ui->passwordlineEdit->text();

    if (password.isEmpty()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body);
    } else if (password.length() < 6) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body + tr("The password is too short."));
    } else if (password != ui->repasswordlineEdit->text()) {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        ui->body->setText(body + tr("The password doesn't match."));
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        ui->body->setText(body);
    }

    ui->passStrengthMeter->setValue(getPasswordStrength(password));
}

int SetPasswordDialog::getPasswordStrength(QString pass)
{
    if (pass.size() < 6)
        return 0;

    double fscore = 0;
    QHash<QChar, int> charCounts;
    for (QChar c : pass) {
        charCounts[c]++;
        fscore += 5. / charCounts[c];
    }

    int variations = -1;
    variations += pass.contains(QRegularExpression("[0-9]")) ? 1 : 0;
    variations += pass.contains(QRegularExpression("[a-z]")) ? 1 : 0;
    variations += pass.contains(QRegularExpression("[A-Z]")) ? 1 : 0;
    variations += pass.contains(QRegularExpression("[\\W]")) ? 1 : 0;

    int score = fscore;
    score += variations * 10;
    score -= 20;
    score = std::min(score, 100);
    score = std::max(score, 0);

    return score;
}

QString SetPasswordDialog::getPassword()
{
    return ui->passwordlineEdit->text();
}
