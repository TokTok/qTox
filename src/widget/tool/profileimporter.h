/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>
class Settings;

class ProfileImporter : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileImporter(Settings& settings, QWidget* parent = nullptr);
    bool importProfile(const QString& path);
    bool importProfile();

private:
    bool askQuestion(QString title, QString message);
    Settings& settings;
};
