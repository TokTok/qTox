/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */
#pragma once

#include "src/video/videomode.h"
#include <QPair>
#include <QString>
#include <QVector>

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
namespace v4l2 {
QVector<VideoMode> getDeviceModes(QString devName);
QVector<QPair<QString, QString>> getDeviceList();
QString getPixelFormatString(uint32_t pixel_format);
bool betterPixelFormat(uint32_t a, uint32_t b);
} // namespace v4l2
#endif
