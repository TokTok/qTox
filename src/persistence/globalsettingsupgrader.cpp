/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "globalsettingsupgrader.h"
#include "src/persistence/settings.h"
#include "src/persistence/paths.h"

#include <QDebug>
#include <QFile>

namespace {
    bool version0to1(Settings& settings)
    {
        const auto& paths = settings.getPaths();

        QFile badFile{paths.getUserNodesFilePath()};
        if (badFile.exists()) {
            if (!badFile.rename(paths.getBackupUserNodesFilePath())) {
                qCritical() << "Failed to write to" << paths.getBackupUserNodesFilePath() << \
                    "aborting bootstrap node upgrade.";
                return false;
            }
        }
        qWarning() << "Moved" << badFile.fileName() << "to" << paths.getBackupUserNodesFilePath() << \
            "to mitigate a bug. Resetting bootstrap nodes to default.";
        return true;
    }
} // namespace

#include <cassert>

bool GlobalSettingsUpgrader::doUpgrade(Settings& settings, int fromVer, int toVer)
{
    if (fromVer == toVer) {
        return true;
    }

    if (fromVer > toVer) {
        qWarning().nospace() << "Settings version (" << fromVer
                             << ") is newer than we currently support (" << toVer
                             << "). Please upgrade qTox";
        return false;
    }

    using SettingsUpgradeFn = bool (*)(Settings&);
    std::vector<SettingsUpgradeFn> upgradeFns = {version0to1};

    assert(fromVer < static_cast<int>(upgradeFns.size()));
    assert(toVer == static_cast<int>(upgradeFns.size()));

    for (int i = fromVer; i < static_cast<int>(upgradeFns.size()); ++i) {
        auto const newSettingsVersion = i + 1;
        if (!upgradeFns[i](settings)) {
            qCritical() << "Failed to upgrade settings to version" << newSettingsVersion << "aborting";
            return false;
        }
        qDebug() << "Settings upgraded incrementally to schema version " << newSettingsVersion;
    }

    qInfo() << "Settings upgrade finished (settingsVersion" << fromVer << "->"
            << toVer << ")";
    return true;
}
