/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "appmanager.h"

#include <QDebug>
#include <QGuiApplication>

int main(int argc, char* argv[])
{
    AppManager appManager(argc, argv);
    int errorcode = appManager.run();

    qDebug() << "Exit with status" << errorcode;
    return errorcode;
}
