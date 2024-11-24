/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */


#pragma once

#include "src/video/videomode.h"
#include <QPair>
#include <QString>
#include <QVector>

#ifdef Q_OS_WIN
namespace DirectShow {
QVector<QPair<QString, QString>> getDeviceList();
QVector<VideoMode> getDeviceModes(QString devName);
} // namespace DirectShow
#endif
