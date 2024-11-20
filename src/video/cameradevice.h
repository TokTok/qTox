/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */


#pragma once

#include "videomode.h"
#include <QHash>
#include <QMutex>
#include <QString>
#include <QVector>
#include <atomic>

struct AVFormatContext;
struct AVInputFormat;
struct AVDeviceInfoList;
struct AVDictionary;
class Settings;

class CameraDevice
{
public:
    static CameraDevice* open(QString devName, VideoMode mode = VideoMode());
    void open();
    bool close();

    static QVector<QPair<QString, QString>> getDeviceList();

    static QVector<VideoMode> getVideoModes(QString devName);
    static QString getPixelFormatString(uint32_t pixel_format);
    static bool betterPixelFormat(uint32_t a, uint32_t b);

    static QString getDefaultDeviceName(Settings& settings);

    static bool isScreen(const QString& devName);

private:
    CameraDevice(QString devName_, AVFormatContext* context_);
    static CameraDevice* open(QString devName, AVDictionary** options);
    static bool getDefaultInputFormat();
    static QVector<QPair<QString, QString>> getRawDeviceListGeneric();
    static QVector<VideoMode> getScreenModes();

public:
    const QString devName;
    AVFormatContext* context;

private:
    std::atomic_int refcount;
    static QHash<QString, CameraDevice*> openDevices;
    static QMutex openDeviceLock, iformatLock;
};
