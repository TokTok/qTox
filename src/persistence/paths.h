/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <atomic>

#define PATHS_VERSION_TCS_COMPLIANT 0

class Paths
{
public:
    enum class Portable
    {
        Auto,       /** Auto detect if portable or non-portable */
        Portable,   /** Force portable mode */
        NonPortable /** Force non-portable mode */
    };

    Paths(Portable mode = Portable::Auto);

    bool setPortable(bool portable);
    bool isPortable() const;

    void setPortablePath(const QString& path);

#if PATHS_VERSION_TCS_COMPLIANT
    QString getGlobalSettingsPath() const;
    QString getProfilesDir() const;
    QString getToxSaveDir() const;
    QString getAvatarsDir() const;
    QString getTransfersDir() const;
    QStringList getThemeDirs() const;
    QString getScreenshotsDir() const;
#else
    // to be removed when paths migration is complete.
    QString getSettingsDirPath() const;
    QString getAppDataDirPath() const;
    QString getAppCacheDirPath() const;
    QString getExampleNodesFilePath() const;
    QString getUserNodesFilePath() const;
    QString getBackupUserNodesFilePath() const;
#endif

private:
    QString basePath;
    QString overridePath;
    std::atomic_bool portable{false};
};
