/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/video/videomode.h"
#include "src/video/videosource.h"

#include <QFuture>
#include <QHash>
#include <QReadWriteLock>
#include <QString>
#include <QVector>

#include <atomic>

class CameraDevice;
struct AVCodecContext;
class Settings;

class CameraSource : public VideoSource
{
    Q_OBJECT

public:
    explicit CameraSource(Settings& settings);
    ~CameraSource() override;
    void setupDefault();
    bool isNone() const;

    // VideoSource interface
    void subscribe() override;
    void unsubscribe() override;

public slots:
    void setupDevice(const QString& deviceName_, const VideoMode& mode_);

signals:
    void deviceOpened();
    void openFailed();

private:
    void stream();

private slots:
    void openDevice();
    void closeDevice();

private:
    QFuture<void> streamFuture;
    QThread* deviceThread;

    QString deviceName;
    CameraDevice* device;
    VideoMode mode;
    AVCodecContext* cctx;
    // TODO: Remove when ffmpeg version will be bumped to the 3.1.0
    AVCodecContext* cctxOrig;
    int videoStreamIndex;

    QReadWriteLock deviceMutex;
    QReadWriteLock streamMutex;

    std::atomic_bool isNone_;
    std::atomic_int subscriptions;
    Settings& settings;
};
