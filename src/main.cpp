/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "appmanager.h"

#include <QDebug>
#include <QGuiApplication>

#ifdef Q_OS_ANDROID
#include <unistd.h>

static volatile bool startTheProgram = false;
#endif

int main(int argc, char* argv[])
{
#ifdef Q_OS_ANDROID
    while (!startTheProgram) {
        // Wait for the debugger to attach
        sleep(1);
    }
#endif
    AppManager appManager(argc, argv);
    int errorcode = appManager.run();

    qDebug() << "Exit with status" << errorcode;
    return errorcode;
}
