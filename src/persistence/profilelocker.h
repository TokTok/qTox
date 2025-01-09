/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */


#pragma once

#include <QLockFile>

#include <memory>

class Paths;

class ProfileLocker
{
public:
    ProfileLocker() = delete;

    static bool isLockable(QString profile, Paths& paths);
    static bool lock(QString profile, Paths& paths);
    static void unlock();
    static bool hasLock();
    static QString getCurLockName();
    static void assertLock(Paths& paths);

private:
    static QString lockPathFromName(const QString& name, const Paths& paths);
    static void deathByBrokenLock();

private:
    static std::unique_ptr<QLockFile> lockfile;
    static QString curLockName;
};
