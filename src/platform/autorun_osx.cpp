/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/platform/autorun.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

namespace {
QString getAutorunFile()
{
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
            + QDir::separator() + "Library" + QDir::separator() + "LaunchAgents"
            + QDir::separator() + "chat.tox.qtox.autorun.plist");
}
} // namespace

bool Platform::setAutorun(const Settings& settings, bool on)
{
    std::ignore = settings;
    QString qtoxDir =
        QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + "qtox");
    QSettings autoRun(getAutorunFile(), QSettings::NativeFormat);
    autoRun.setValue("Label", "chat.tox.qtox.autorun");
    autoRun.setValue("Program", qtoxDir);

    autoRun.setValue("RunAtLoad", on);
    return true;
}

bool Platform::getAutorun(const Settings& settings)
{
    std::ignore = settings;
    QSettings autoRun(getAutorunFile(), QSettings::NativeFormat);
    return autoRun.value("RunAtLoad", false).toBool();
}
