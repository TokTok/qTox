/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014 Thilo Borgmann <thilo.borgmann@mail.de>
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "avfoundation.h"

#include <QDebug>
#include <QMutex>
#include <QObject>

#import <AVFoundation/AVFoundation.h>

namespace {

NSArray* getDevices()
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500
    NSMutableArray* deviceTypes =
        [NSMutableArray arrayWithArray:@[ AVCaptureDeviceTypeBuiltInWideAngleCamera ]];
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 130000
    [deviceTypes addObject:AVCaptureDeviceTypeDeskViewCamera];
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 140000
    [deviceTypes addObject:AVCaptureDeviceTypeContinuityCamera];
    [deviceTypes addObject:AVCaptureDeviceTypeExternal];
#else
    [deviceTypes addObject:AVCaptureDeviceTypeExternalUnknown];
#endif

    AVCaptureDeviceDiscoverySession* captureDeviceDiscoverySession = [AVCaptureDeviceDiscoverySession
        discoverySessionWithDeviceTypes:deviceTypes
                              mediaType:AVMediaTypeVideo
                               position:AVCaptureDevicePositionUnspecified];
    return [captureDeviceDiscoverySession devices];
#else
    return [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
#endif
}

} // namespace

bool avfoundation::isDesktopCapture(const QString& devName)
{
    NSArray* devices = getDevices();
    const int index = devName.toInt();
    return index >= [devices count];
}

bool avfoundation::hasPermission(const QString& devName)
{
    if (isDesktopCapture(devName)) {
        return CGPreflightScreenCaptureAccess();
    }

    const AVAuthorizationStatus authStatus =
        [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    return authStatus == AVAuthorizationStatusAuthorized;
}

QVector<QPair<QString, QString>> avfoundation::getDeviceList()
{
    QVector<QPair<QString, QString>> result;

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101400
    CGRequestScreenCaptureAccess();
    const AVAuthorizationStatus authStatus =
        [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    if (authStatus != AVAuthorizationStatusDenied && authStatus != AVAuthorizationStatusNotDetermined) {
        qDebug() << "We already have access to the camera.";
    } else {
        qDebug() << "We don't have access to the camera yet; asking user for permission.";
        QMutex mutex;
        mutex.lock();
        QMutex* mutexPtr = &mutex;
        __block BOOL isGranted = false;
        [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo
                                 completionHandler:^(BOOL granted) {
                                   isGranted = granted;
                                   mutexPtr->unlock();
                                 }];
        // Lock it again so the callback can unlock it and we can proceed.
        mutex.lock();
        if (isGranted) {
            qInfo() << "We now have access to the camera.";
        } else {
            qInfo() << "User did not grant us permission to access the camera.";
        }
    }
#endif

    NSArray* devices = getDevices();

    for (AVCaptureDevice* device in devices) {
        int index = [devices indexOfObject:device];
        result.append({QString::number(index), QString::fromNSString([device localizedName])});
    }

    uint32_t numScreens = 0;
    CGGetActiveDisplayList(0, nullptr, &numScreens);
    if (numScreens > 0) {
        CGDirectDisplayID screens[numScreens];
        CGGetActiveDisplayList(numScreens, screens, &numScreens);
        for (uint32_t i = 0; i < numScreens; i++) {
            result.append({QString::number(result.size()), QObject::tr("Capture screen %1").arg(i)});
        }
    }

    return result;
}

QVector<VideoMode> avfoundation::getDeviceModes(const QString& devName)
{
    QVector<VideoMode> result;
    NSArray* devices = getDevices();
    const int index = devName.toInt();

    if (index >= [devices count]) {
        // It's a desktop capture.
        return result;
    } else {
        AVCaptureDevice* device = [devices objectAtIndex:index];

        if (device == nil) {
            qWarning() << "Device not found:" << devName;
            return result;
        }

        for (AVCaptureDeviceFormat* format in [device formats]) {
            CMFormatDescriptionRef formatDescription = static_cast<CMFormatDescriptionRef>(
                [format performSelector:@selector(formatDescription)]);
            CMVideoDimensions dimensions = CMVideoFormatDescriptionGetDimensions(formatDescription);

            for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges) {
                VideoMode mode;
                mode.width = dimensions.width;
                mode.height = dimensions.height;
                mode.fps = range.maxFrameRate;
                result.append(mode);
            }
        }
    }

    return result;
}
