/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "removechatdialog.h"
#include "src/model/chat.h"

#include <QPushButton>


RemoveChatDialog::RemoveChatDialog(QWidget* parent, const Chat& contact)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_QuitOnClose, false);
    ui.setupUi(this);
    const QString name = contact.getDisplayedName().toHtmlEscaped();
    const QString text = tr("Are you sure you want to remove %1 from your contacts list?")
                             .arg(QString("<b>%1</b>").arg(name));

    ui.label->setText(text);
    auto removeButton = ui.buttonBox->button(QDialogButtonBox::Ok);
    auto cancelButton = ui.buttonBox->button(QDialogButtonBox::Cancel);
    removeButton->setText(tr("Remove"));
    cancelButton->setDefault(true);
    adjustSize();
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &RemoveChatDialog::onAccepted);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &RemoveChatDialog::close);
    setFocus();
}

void RemoveChatDialog::onAccepted()
{
    _accepted = true;
    close();
}
