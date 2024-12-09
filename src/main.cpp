/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "appmanager.h"

#include <QDebug>
#include <QGuiApplication>
#include <QtPlugin>

int main(int argc, char* argv[])
{
    AppManager appManager(argc, argv);
    int errorcode = appManager.run();

    qDebug() << "Exit with status" << errorcode;
    return errorcode;
}

#ifdef QT_STATIC
Q_IMPORT_PLUGIN(QLinuxFbIntegrationPlugin)
Q_IMPORT_PLUGIN(QOffscreenIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbGlxIntegrationPlugin)
Q_IMPORT_PLUGIN(QVncIntegrationPlugin)
Q_IMPORT_PLUGIN(QWaylandIntegrationPlugin)
#endif
