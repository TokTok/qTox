/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */
#pragma once

#include "src/video/videomode.h"

#include <QPair>
#include <QString>
#include <QVector>

#ifdef Q_OS_WASM
struct AVDictionary;
struct AVFormatContext;

namespace browser {
QVector<QPair<QString, QString>> getDeviceList();
QVector<VideoMode> getDeviceModes();
AVFormatContext* openCamera(AVDictionary** options);
void closeCamera(AVFormatContext** context);
} // namespace browser
#endif
