/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include <QtCore/qsystemdetection.h>
#include "src/platform/x11_display.h"
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
        if (display) {
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
}
