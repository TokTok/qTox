/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

class QString;
class QFileInfo;

class IMessageBoxManager
{
public:
    virtual ~IMessageBoxManager();
    virtual void showInfo(const QString& title, const QString& msg) = 0;
    virtual void showWarning(const QString& title, const QString& msg) = 0;
    virtual void showError(const QString& title, const QString& msg) = 0;
    virtual bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
            bool warning = true, bool yesno = true) = 0;
    virtual bool askQuestion(const QString& title, const QString& msg, const QString& button1,
            const QString& button2, bool defaultAns = false, bool warning = true) = 0;
    virtual void confirmExecutableOpen(const QFileInfo& file) = 0;
};
