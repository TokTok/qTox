/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/platform/capslock.h"
#include <QtCore/qsystemdetection.h>

// TODO: Implement for osx
bool Platform::capsLockEnabled()
{
    return false;
}
