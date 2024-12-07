/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "capslock.h"

#ifdef QTOX_PLATFORM_EXT
#include <QtCore/qsystemdetection.h>

#ifdef Q_OS_MACOS
#include <QDebug>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>

bool Platform::capsLockEnabled()
{
    const auto mdict = IOServiceMatching(kIOHIDSystemClass);
    const auto ios = IOServiceGetMatchingService(kIOMainPortDefault, mdict);
    if (!ios) {
        if (mdict) {
            CFRelease(mdict);
        }
        qWarning("IOServiceGetMatchingService() failed");
        return false;
    }

    io_connect_t ioc;
    auto kr = IOServiceOpen(ios, mach_task_self(), kIOHIDParamConnectType, &ioc);
    IOObjectRelease(ios);
    if (kr != KERN_SUCCESS) {
        qWarning("IOServiceOpen() failed: %x", kr);
        return false;
    }

    bool state;
    kr = IOHIDGetModifierLockState(ioc, kIOHIDCapsLockState, &state);
    if (kr != KERN_SUCCESS) {
        IOServiceClose(ioc);
        qWarning("IOHIDGetModifierLockState() failed: %x", kr);
        return false;
    }
    IOServiceClose(ioc);
    return state;
}
#endif
#endif
