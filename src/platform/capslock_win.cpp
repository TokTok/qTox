/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/platform/capslock.h"

#include <QtCore/qsystemdetection.h>

#ifdef Q_OS_WIN
#include <windows.h>

bool Platform::capsLockEnabled()
{
    return GetKeyState(VK_CAPITAL) == 1;
}
#endif
