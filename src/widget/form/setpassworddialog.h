/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QDialog>

namespace Ui {
class SetPasswordDialog;
}

class SetPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    enum ReturnCode
    {
        Rejected = QDialog::Rejected,
        Accepted = QDialog::Accepted,
        Tertiary
    };
    explicit SetPasswordDialog(QString body_, QString extraButton, QWidget* parent = nullptr);
    ~SetPasswordDialog();
    QString getPassword();
    static int getPasswordStrength(QString pass);

private slots:
    void onPasswordEdit();

private:
    Ui::SetPasswordDialog* ui;
    QString body;
    static const double reasonablePasswordLength;
};
