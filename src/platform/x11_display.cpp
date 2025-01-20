/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/platform/x11_display.h"

#ifdef QTOX_PLATFORM_EXT
#include <QtCore/qsystemdetection.h>

#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
#include <QMutex>
#include <X11/Xlib.h>

namespace Platform {

struct X11DisplayPrivate
{
    Display* display;
    QMutex mutex;

    X11DisplayPrivate()
        : display(XOpenDisplay(nullptr))
    {
    }
    ~X11DisplayPrivate()
    {
        if (display != nullptr) {
            XCloseDisplay(display);
        }
    }
    static X11DisplayPrivate& getSingleInstance()
    {
        // object created on-demand
        static X11DisplayPrivate singleInstance;
        return singleInstance;
    }
};

Display* X11Display::lock()
{
    X11DisplayPrivate& singleInstance = X11DisplayPrivate::getSingleInstance();
    singleInstance.mutex.lock();
    return singleInstance.display;
}

void X11Display::unlock()
{
    X11DisplayPrivate::getSingleInstance().mutex.unlock();
}
} // namespace Platform
#endif
#endif
