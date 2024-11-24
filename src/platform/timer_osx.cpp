/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2006 by Richard Laager
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution (which can be found at
 * <https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/COPYRIGHT> ).
 */

#include "src/platform/timer.h"

#include <QtCore/qsystemdetection.h>

#ifdef Q_OS_MACOS
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

uint32_t Platform::getIdleTime()
{
    // https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/pidgin/gtkidle.c
    // relevant code introduced to Pidgin in:
    // https://hg.pidgin.im/pidgin/main/diff/8ff1c408ef3e/src/gtkidle.c
    static io_service_t service = 0;
    CFTypeRef property;
    uint64_t idleTime_ns = 0;

    if (!service) {
        mach_port_t master;
        IOMainPort(MACH_PORT_NULL, &master);
        service = IOServiceGetMatchingService(master, IOServiceMatching("IOHIDSystem"));
    }

    property = IORegistryEntryCreateCFProperty(service, CFSTR("HIDIdleTime"), kCFAllocatorDefault, 0);
    CFNumberGetValue(static_cast<CFNumberRef>(property), kCFNumberSInt64Type, &idleTime_ns);
    CFRelease(property);

    return idleTime_ns / 1000000;
}
#endif
