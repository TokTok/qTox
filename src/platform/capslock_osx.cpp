/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "capslock.h"

#include <QtCore/qsystemdetection.h>

#ifdef Q_OS_MACOS
// TODO: Implement for osx
bool Platform::capsLockEnabled()
{
    return false;
}
#endif
