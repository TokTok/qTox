/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QObject>
#include <QWidget>

#include "src/widget/tool/imessageboxmanager.h"

class QFileInfo;

class MessageBoxManager : public QWidget, public IMessageBoxManager
{
Q_OBJECT
public:
    explicit MessageBoxManager(QWidget* parent);
    ~MessageBoxManager() override = default;
    void showInfo(const QString& title, const QString& msg) override;
    void showWarning(const QString& title, const QString& msg) override;
    void showError(const QString& title, const QString& msg) override;
    bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                            bool warning = true, bool yesno = true) override;
    bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                            const QString& button2, bool defaultAns = false, bool warning = true) override;
    void confirmExecutableOpen(const QFileInfo& file) override;

private slots:
    // Private implementation, those must be called from the GUI thread
    void _showInfo(const QString& title, const QString& msg);
    void _showWarning(const QString& title, const QString& msg);
    void _showError(const QString& title, const QString& msg);
    bool _askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                      bool warning = true, bool yesno = true);
    bool _askQuestion(const QString& title, const QString& msg, const QString& button1,
                      const QString& button2, bool defaultAns = false, bool warning = true);
};
