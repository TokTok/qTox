/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/persistence/settings.h"
#include "src/platform/autorun.h"
#include <QApplication>
#include <QDir>
#include <QProcessEnvironment>

namespace {
QString getAutostartDirPath()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString config = env.value("XDG_CONFIG_HOME");
    if (config.isEmpty())
        config = QDir::homePath() + "/" + ".config";
    return config + "/autostart";
}

QString getAutostartFilePath(const Settings& settings, QString dir)
{
    return dir + "/qTox - " + settings.getCurrentProfile() + ".desktop";
}

QString currentBinPath()
{
    const auto env = QProcessEnvironment::systemEnvironment();
    const auto appImageEnvKey = QStringLiteral("APPIMAGE");

    if (env.contains(appImageEnvKey)) {
        return env.value(appImageEnvKey);
    } else {
        return QApplication::applicationFilePath();
    }
}

inline QString profileRunCommand(const Settings& settings)
{
    return "\"" + currentBinPath() + "\" -p \"" + settings.getCurrentProfile() + "\"";
}
} // namespace

bool Platform::setAutorun(const Settings& settings, bool on)
{
    QString dirPath = getAutostartDirPath();
    QFile desktop(getAutostartFilePath(settings, dirPath));
    if (on) {
        if (!QDir().mkpath(dirPath) || !desktop.open(QFile::WriteOnly | QFile::Truncate))
            return false;
        desktop.write("[Desktop Entry]\n");
        desktop.write("Type=Application\n");
        desktop.write("Name=qTox\n");
        desktop.write("Exec=");
        desktop.write(profileRunCommand(settings).toUtf8());
        desktop.write("\n");
        desktop.close();
        return true;
    } else
        return desktop.remove();
}

bool Platform::getAutorun(const Settings& settings)
{
    return QFile(getAutostartFilePath(settings, getAutostartDirPath())).exists();
}
