/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/platform/timer.h"
#include <QtCore/qsystemdetection.h>

#ifdef Q_OS_WIN
#include <windows.h>

uint32_t Platform::getIdleTime()
{
    LASTINPUTINFO info = {0, 0};
    info.cbSize = sizeof(info);
    if (GetLastInputInfo(&info))
        return GetTickCount() - info.dwTime;
    return 0;
}
#endif
