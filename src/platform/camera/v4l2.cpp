/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2000,2001 Fabrice Bellard
 * Copyright © 2006 Luca Abeni
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */


#include "v4l2.h"

#if defined(USING_V4L)
#include <QDebug>
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <map>
#include <sys/ioctl.h>
#include <unistd.h>

/**
 * Most of this file is adapted from libavdevice's v4l2.c,
 * which retrieves useful information but only exposes it to
 * stdout and is not part of the public API for some reason.
 */

namespace {
std::map<uint32_t, uint8_t> createPixFmtToQuality()
{
    std::map<uint32_t, uint8_t> m;
    m[V4L2_PIX_FMT_H264] = 3;
    m[V4L2_PIX_FMT_MJPEG] = 2;
    m[V4L2_PIX_FMT_YUYV] = 1;
    m[V4L2_PIX_FMT_UYVY] = 1;
    return m;
}
const std::map<uint32_t, uint8_t> pixFmtToQuality = createPixFmtToQuality();

std::map<uint32_t, QString> createPixFmtToName()
{
    std::map<uint32_t, QString> m;
    m[V4L2_PIX_FMT_H264] = QString("h264");
    m[V4L2_PIX_FMT_MJPEG] = QString("mjpeg");
    m[V4L2_PIX_FMT_YUYV] = QString("yuyv422");
    m[V4L2_PIX_FMT_UYVY] = QString("uyvy422");
    return m;
}
const std::map<uint32_t, QString> pixFmtToName = createPixFmtToName();

int deviceOpen(QString devName, int* error)
{
    struct v4l2_capability cap;
    int fd;

    const std::string devNameString = devName.toStdString();
    fd = open(devNameString.c_str(), O_RDWR, 0);
    if (fd < 0) {
        *error = errno;
        return fd;
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        *error = errno;
        goto fail;
    }

    if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0u) {
        *error = ENODEV;
        goto fail;
    }

    if ((cap.capabilities & V4L2_CAP_STREAMING) == 0u) {
        *error = ENOSYS;
        goto fail;
    }

    return fd;

fail:
    close(fd);
    return -1;
}

QVector<float> getDeviceModeFrameRates(int fd, unsigned w, unsigned h, uint32_t pixelFormat)
{
    QVector<float> rates;
    v4l2_frmivalenum vfve{};
    vfve.pixel_format = pixelFormat;
    vfve.height = h;
    vfve.width = w;

    while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &vfve) == 0) {
        float rate;
        switch (vfve.type) {
        case V4L2_FRMSIZE_TYPE_DISCRETE:
            rate = vfve.discrete.denominator / vfve.discrete.numerator;
            if (!rates.contains(rate))
                rates.append(rate);
            break;
        case V4L2_FRMSIZE_TYPE_CONTINUOUS:
        case V4L2_FRMSIZE_TYPE_STEPWISE:
            rate = vfve.stepwise.min.denominator / vfve.stepwise.min.numerator;
            if (!rates.contains(rate))
                rates.append(rate);
        }
        vfve.index++;
    }

    return rates;
}
} // namespace

QVector<VideoMode> v4l2::getDeviceModes(QString devName)
{
    QVector<VideoMode> modes;

    int error = 0;
    const int fd = deviceOpen(devName, &error);
    if (fd < 0 || error != 0) {
        return modes;
    }

    v4l2_fmtdesc vfd{};
    vfd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &vfd) == 0) {
        vfd.index++;

        v4l2_frmsizeenum vfse{};
        vfse.pixel_format = vfd.pixelformat;

        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &vfse) == 0) {
            VideoMode mode;
            mode.pixel_format = vfse.pixel_format;
            switch (vfse.type) {
            case V4L2_FRMSIZE_TYPE_DISCRETE:
                mode.width = vfse.discrete.width;
                mode.height = vfse.discrete.height;
                break;
            case V4L2_FRMSIZE_TYPE_CONTINUOUS:
            case V4L2_FRMSIZE_TYPE_STEPWISE:
                mode.width = vfse.stepwise.max_width;
                mode.height = vfse.stepwise.max_height;
                break;
            default:
                continue;
            }

            QVector<float> rates =
                getDeviceModeFrameRates(fd, mode.width, mode.height, vfd.pixelformat);

            // insert dummy FPS value to have the mode in the list even if we don't know the FPS
            // this fixes support for some webcams, see #5082
            if (rates.isEmpty()) {
                rates.append(0.0f);
            }

            for (const float rate : rates) {
                mode.fps = rate;
                if (!modes.contains(mode)) {
                    modes.append(mode);
                }
            }
            vfse.index++;
        }
    }

    close(fd);
    return modes;
}

QVector<QPair<QString, QString>> v4l2::getDeviceList()
{
    QVector<QPair<QString, QString>> devices;
    QStringList deviceFiles;

    DIR* dir = opendir("/dev");
    if (dir == nullptr)
        return devices;

    dirent* e;
    while ((e = readdir(dir)) != nullptr)
        if ((strncmp(e->d_name, "video", 5) == 0) || (strncmp(e->d_name, "vbi", 3) == 0))
            deviceFiles += QString("/dev/") + QString::fromUtf8(e->d_name);
    closedir(dir);

    for (const QString& file : deviceFiles) {
        const std::string filePath = file.toStdString();
        const int fd = open(filePath.c_str(), O_RDWR);
        if (fd < 0) {
            continue;
        }

        v4l2_capability caps;
        ioctl(fd, VIDIOC_QUERYCAP, &caps);
        close(fd);

        if ((caps.device_caps & V4L2_CAP_VIDEO_CAPTURE) != 0u)
            devices += {file, QString::fromUtf8(reinterpret_cast<const char*>(caps.card))};
    }
    return devices;
}

QString v4l2::getPixelFormatString(uint32_t pixel_format)
{
    if (!pixFmtToName.contains(pixel_format)) {
        qWarning() << "Pixel format not found";
        return QStringLiteral("invalid");
    }
    return pixFmtToName.at(pixel_format);
}

bool v4l2::betterPixelFormat(uint32_t a, uint32_t b)
{
    if (!pixFmtToQuality.contains(a)) {
        return false;
    }
    if (!pixFmtToQuality.contains(b)) {
        return true;
    }
    return pixFmtToQuality.at(a) > pixFmtToQuality.at(b);
}
#endif
