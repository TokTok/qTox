/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/platform/timer.h"

#ifdef QTOX_PLATFORM_EXT
#include <QtCore/qsystemdetection.h>

#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
#include "src/platform/x11_display.h"
#include <QDebug>
#include <X11/extensions/scrnsaver.h>

uint32_t Platform::getIdleTime()
{
    uint32_t idleTime = 0;

    Display* display = X11Display::lock();
    if (display == nullptr) {
        qDebug() << "XOpenDisplay failed";
        X11Display::unlock();
        return 0;
    }

    int32_t x11event = 0;
    int32_t x11error = 0;
    static const int32_t hasExtension = XScreenSaverQueryExtension(display, &x11event, &x11error);
    if (hasExtension != 0) {
        XScreenSaverInfo* info = XScreenSaverAllocInfo();
        if (info != nullptr) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
            XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
#pragma GCC diagnostic pop
            idleTime = info->idle;
            XFree(info);
        } else
            qDebug() << "XScreenSaverAllocInfo() failed";
    }
    X11Display::unlock();
    return idleTime;
}
#endif
#endif
