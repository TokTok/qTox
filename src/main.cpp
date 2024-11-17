/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "appmanager.h"

#include <QDebug>
#include <QGuiApplication>
#include <QtPlugin>

int main(int argc, char* argv[])
{
#if defined(QT_STATIC) && defined(SPELL_CHECKING)
    Q_INIT_RESOURCE(trigrams);
#endif
    AppManager appManager(argc, argv);
    const int errorcode = appManager.run();

    qDebug() << "Exit with status" << errorcode;
    return errorcode;
}

#ifdef QT_STATIC
// All platforms support offscreen rendering for testing.
Q_IMPORT_PLUGIN(QOffscreenIntegrationPlugin)

#if defined(Q_OS_LINUX)
Q_IMPORT_PLUGIN(QLinuxFbIntegrationPlugin)
Q_IMPORT_PLUGIN(QVncIntegrationPlugin)
Q_IMPORT_PLUGIN(QWaylandIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#elif defined(Q_OS_MACOS)
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#else
#error "No static linking supported for platform"
#endif

#ifdef QTOX_USE_KIMG_QOI
Q_IMPORT_PLUGIN(QOIPlugin)
#endif
#endif // QT_STATIC
