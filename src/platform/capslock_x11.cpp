/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/platform/capslock.h"

#include <QtCore/qsystemdetection.h>

#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
#include "src/platform/x11_display.h"
#include <X11/XKBlib.h>
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

bool Platform::capsLockEnabled()
{
    Display* d = X11Display::lock();
    bool caps_state = false;
    if (d) {
        unsigned n;
        XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
        caps_state = (n & 0x01) == 1;
    }
    X11Display::unlock();
    return caps_state;
}
#endif
