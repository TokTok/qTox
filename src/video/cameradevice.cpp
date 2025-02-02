/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include <QApplication>
#include <QDebug>
#include <QScreen>
extern "C"
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#pragma GCC diagnostic pop
}
#include "cameradevice.h"
#include "scopedavdictionary.h"

#include "src/persistence/settings.h"

#include <QMutex>
#include <QMutexLocker>
#include <QVector>

#include <utility>

// no longer needed when avformat version < 59 is no longer supported
using AvFindInputFormatRet = decltype(av_find_input_format(""));

#include "src/platform/camera/avfoundation.h" // IWYU pragma: keep
#include "src/platform/camera/directshow.h"   // IWYU pragma: keep
#include "src/platform/camera/v4l2.h"         // IWYU pragma: keep

/**
 * @class CameraDevice
 *
 * Maintains an FFmpeg context for open camera devices,
 * takes care of sharing the context across users and closing
 * the camera device when not in use. The device can be opened
 * recursively, and must then be closed recursively
 */


/**
 * @var const QString CameraDevice::devName
 * @brief Short name of the device
 *
 * @var AVFormatContext* CameraDevice::context
 * @brief Context of the open device, must always be valid
 *
 * @var std::atomic_int CameraDevice::refcount;
 * @brief Number of times the device was opened
 */

namespace {
AvFindInputFormatRet idesktopFormat{nullptr};
AvFindInputFormatRet iformat{nullptr};

QDebug operator<<(QDebug dbg, const AVDictionary* const* dict)
{
    QVector<QString> options;
    AVDictionaryEntry* entry = nullptr;
    while ((entry = av_dict_get(*dict, "", entry, AV_DICT_IGNORE_SUFFIX)) != nullptr) {
        options.append(QStringLiteral("%1: %2").arg(QString::fromUtf8(entry->key),
                                                    QString::fromUtf8(entry->value)));
    }

    return dbg << options.join(", ").toUtf8().constData();
}

constexpr int numDigits(uint32_t num)
{
    int digits = 0;
    while (num != 0u) {
        num /= 10;
        ++digits;
    }
    return digits;
}

template <uint32_t Num>
constexpr auto toCharArray()
{
    std::array<char, numDigits(Num) + 1> str{};
    int cur = numDigits(Num);
    for (uint32_t num = Num; num; num /= 10) {
        str[--cur] = num % 10 + '0';
    }
    return str;
}

// Compile-time unit test for the above function.
static_assert(toCharArray<12345>() == std::array<char, 6>{'1', '2', '3', '4', '5', '\0'});
} // namespace

QHash<QString, CameraDevice*> CameraDevice::openDevices;
QMutex CameraDevice::openDeviceLock, CameraDevice::iformatLock;

CameraDevice::CameraDevice(QString devName_, AVFormatContext* context_)
    : devName{std::move(devName_)}
    , context{context_}
    , refcount{1}
{
}

CameraDevice* CameraDevice::open(QString devName, AVDictionary** options)
{
    const QMutexLocker<QMutex> lock(&openDeviceLock);
    CameraDevice* dev = openDevices.value(devName);
    if (dev != nullptr) {
        return dev;
    }

    const AvFindInputFormatRet format = [&devName] {
        for (const auto& grabName : {QStringLiteral("x11grab#"), QStringLiteral("gdigrab#")}) {
            if (devName.startsWith(grabName)) {
                devName = devName.mid(grabName.size());
                return idesktopFormat;
            }
        }
        return iformat;
    }();

    qDebug() << "Opening device" << devName << "with format" << format->name << "and options"
             << options;
    AVFormatContext* fctx = nullptr;
    if (avformat_open_input(&fctx, devName.toStdString().c_str(), format, options) < 0) {
        return nullptr;
    }

// Fix avformat_find_stream_info hanging on garbage input
#if defined FF_API_PROBESIZE_32 && FF_API_PROBESIZE_32
    const int aDuration = fctx->max_analyze_duration2;
    fctx->max_analyze_duration2 = 0;
#else
    const int aDuration = fctx->max_analyze_duration;
    fctx->max_analyze_duration = 0;
#endif

    if (avformat_find_stream_info(fctx, nullptr) < 0) {
        avformat_close_input(&fctx);
        return nullptr;
    }

#if defined FF_API_PROBESIZE_32 && FF_API_PROBESIZE_32
    fctx->max_analyze_duration2 = aDuration;
#else
    fctx->max_analyze_duration = aDuration;
#endif

    dev = new CameraDevice{devName, fctx};
    openDevices[devName] = dev;

    return dev;
}

/**
 * @brief Opens a device.
 *
 * Opens a device, creating a new one if needed
 * If the device is already open in another mode, the mode
 * will be ignored and the existing device is used
 * If the mode does not exist, a new device can't be opened.
 *
 * @param devName Device name to open.
 * @param mode Mode of device to open.
 * @return CameraDevice if the device could be opened, nullptr otherwise.
 */
CameraDevice* CameraDevice::open(QString devName, VideoMode mode)
{
    if (!getDefaultInputFormat())
        return nullptr;

    if (devName == "none") {
        qDebug() << "Tried to open the null device";
        return nullptr;
    }

#if defined(USING_V4L) || defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    float fps = 5;
    if (mode.fps > 0.0f) {
        fps = mode.fps;
    } else {
        qWarning() << "Frame rate not set; VideoMode could be invalid (defaulting to" << fps << "fps)";
    }

    const QString videoSize = QStringLiteral("%1x%2").arg(mode.width).arg(mode.height);
    const QString framerate = QString::number(static_cast<double>(fps));
#endif

    ScopedAVDictionary options;
    if (iformat == nullptr)
        ;
#if defined(USING_V4L)
    else if (devName.startsWith("x11grab#")) {
        QSize screen;
        if ((mode.width != 0) && (mode.height != 0)) {
            screen.setWidth(mode.width);
            screen.setHeight(mode.height);
        } else {
            QScreen* defaultScreen = QApplication::primaryScreen();
            const qreal pixRatio = defaultScreen->devicePixelRatio();

            screen = defaultScreen->size();
            // Workaround https://trac.ffmpeg.org/ticket/4574 by choping 1 px bottom and right
            // Actually, let's chop two pixels, toxav hates odd resolutions (off by one stride)
            screen.setWidth((screen.width() * pixRatio) - 2);
            screen.setHeight((screen.height() * pixRatio) - 2);
        }
        const QString screenVideoSize =
            QStringLiteral("%1x%2").arg(screen.width()).arg(screen.height());
        options["video_size"] = screenVideoSize;
        devName += QStringLiteral("+%1,%2").arg(QString::number(mode.x), QString::number(mode.y));

        options["framerate"] = framerate;
    } else if (QString::fromUtf8(iformat->name) == QStringLiteral("video4linux2,v4l2")
               && !mode.isUnspecified()) {
        options["video_size"] = videoSize;
        options["framerate"] = framerate;
        const QString pixelFormatStr = v4l2::getPixelFormatString(mode.pixel_format);
        // don't try to set a format string that doesn't exist
        if (pixelFormatStr != QStringLiteral("unknown") && pixelFormatStr != QStringLiteral("invalid")) {
            options["pixel_format"] = pixelFormatStr;
        }
    }
#endif
#ifdef Q_OS_WIN
    else if (devName.startsWith("gdigrab#")) {
        options["framerate"] = framerate;
        options["video_size"] = videoSize;
        options["offset_x"] = QString::number(mode.x);
        options["offset_y"] = QString::number(mode.y);
    } else if (QString::fromUtf8(iformat->name) == QStringLiteral("dshow") && !mode.isUnspecified()) {
        options["framerate"] = framerate;
        options["video_size"] = videoSize;
    }
#endif
#ifdef Q_OS_MACOS
    else if (QString::fromUtf8(iformat->name) == QStringLiteral("avfoundation")) {
        if (!avfoundation::hasPermission(devName)) {
            qWarning() << "No permission to access device" << devName;
            return nullptr;
        }
        if (avfoundation::isDesktopCapture(devName)) {
            qDebug() << "Opening desktop capture" << devName;
            options["framerate"] = framerate;
            options["capture_cursor"] = 1;
            options["capture_mouse_clicks"] = 1;
        } else if (!mode.isUnspecified()) {
            qDebug() << "Opening camera" << devName;
            options["video_size"] = videoSize;
            options["framerate"] = framerate;
        }
        // We need a fair amount of probe buffer to calculate the fps: 30MiB.
        constexpr auto probesize = toCharArray<30 * 1024 * 1024>();
        options["probesize"] = probesize;
        // TODO(iphydf): Find available pixel formats and select the first one.
        options["pixel_format"] = "uyvy422";
    }
#endif
    else if (!mode.isUnspecified()) {
        qWarning().nospace() << "No known options for " << iformat->name << ", using defaults.";
    }

    return open(devName, options.get());
}

/**
 * @brief Opens the device again. Never fails
 */
void CameraDevice::open()
{
    ++refcount;
}

/**
 * @brief Closes the device. Never fails.
 * @note If returns true, "this" becomes invalid.
 * @return True, if device finally deleted (closed last reference),
 * false otherwise (if other references exist).
 */
bool CameraDevice::close()
{
    if (--refcount > 0)
        return false;

    openDeviceLock.lock();
    openDevices.remove(devName);
    openDeviceLock.unlock();
    avformat_close_input(&context);
    delete this;
    return true;
}

/**
 * @brief Get raw device list
 * @note Uses avdevice_list_devices
 * @return Raw device list
 */
QVector<QPair<QString, QString>> CameraDevice::getRawDeviceListGeneric()
{
    if (!getDefaultInputFormat()) {
        return {};
    }

    if ((iformat->priv_class == nullptr) || !AV_IS_INPUT_DEVICE(iformat->priv_class->category)) {
        return {};
    }

    AVDeviceInfoList* devlist = nullptr;
    AVDictionary* tmp = nullptr;
    avdevice_list_input_sources(iformat, nullptr, tmp, &devlist);
    av_dict_free(&tmp);

    if (devlist == nullptr) {
        qWarning() << "avdevice_list_input_sources failed";
        return {};
    }

    // Convert the list to a QVector
    QVector<QPair<QString, QString>> devices;
    for (int i = 0; i < devlist->nb_devices; ++i) {
        const AVDeviceInfo* dev = devlist->devices[i];
        devices.append({
            QString::fromUtf8(dev->device_name),
            QString::fromUtf8(dev->device_description),
        });
    }
    avdevice_free_list_devices(&devlist);
    return devices;
}

/**
 * @brief Get device list with description
 * @return A list of device names and descriptions.
 * The names are the first part of the pair and can be passed to open(QString).
 */
QVector<QPair<QString, QString>> CameraDevice::getDeviceList()
{
    QVector<QPair<QString, QString>> devices{{"none", QObject::tr("None", "No camera device set")}};

    if (!getDefaultInputFormat())
        return devices;

    if (iformat == nullptr)
        ;
#if defined(USING_V4L)
    else if (QString::fromUtf8(iformat->name) == QStringLiteral("video4linux2,v4l2"))
        devices += v4l2::getDeviceList();
#endif
#ifdef Q_OS_WIN
    else if (QString::fromUtf8(iformat->name) == QStringLiteral("dshow"))
        devices += DirectShow::getDeviceList();
#endif
#ifdef Q_OS_MACOS
    else if (QString::fromUtf8(iformat->name) == QStringLiteral("avfoundation"))
        devices += avfoundation::getDeviceList();
#endif
    else
        devices += getRawDeviceListGeneric();

    if (idesktopFormat != nullptr) {
        if (QString::fromUtf8(idesktopFormat->name) == QStringLiteral("x11grab")) {
            QString dev = "x11grab#";
            const QByteArray display = qgetenv("DISPLAY");

            if (display.isNull())
                dev += ":0";
            else
                dev += QString::fromUtf8(display);

            devices.push_back(QPair<QString, QString>{
                dev, QObject::tr("Desktop", "Desktop as a camera input for screen sharing")});
        }
        if (QString::fromUtf8(idesktopFormat->name) == QStringLiteral("gdigrab"))
            devices.push_back(QPair<QString, QString>{
                "gdigrab#desktop",
                QObject::tr("Desktop", "Desktop as a camera input for screen sharing")});
    }

    return devices;
}

/**
 * @brief Get the default device name.
 * @return The short name of the default device
 * This is either the device in the settings or the system default.
 */
QString CameraDevice::getDefaultDeviceName(Settings& settings)
{
    QString defaultdev = settings.getVideoDev();

    if (!getDefaultInputFormat())
        return defaultdev;

    QVector<QPair<QString, QString>> devlist = getDeviceList();
    for (const QPair<QString, QString>& device : devlist)
        if (defaultdev == device.first)
            return defaultdev;

    if (devlist.isEmpty())
        return defaultdev;

    return devlist[0].first;
}

/**
 * @brief Checks if a device name specifies a display.
 * @param devName Device name to check.
 * @return True, if device is screen, false otherwise.
 */
bool CameraDevice::isScreen(const QString& devName)
{
    return devName.startsWith("x11grab") || devName.startsWith("gdigrab");
}

/**
 * @brief Get list of resolutions and position of screens
 * @return Vector of available screen modes with offset
 */
QVector<VideoMode> CameraDevice::getScreenModes()
{
    QList<QScreen*> screens = QApplication::screens();
    QVector<VideoMode> result;

    std::for_each(screens.begin(), screens.end(), [&result](QScreen* s) {
        const QRect rect = s->geometry();
        const QPoint p = rect.topLeft();
        const qreal pixRatio = s->devicePixelRatio();

        const VideoMode mode(rect.width() * pixRatio, rect.height() * pixRatio, p.x() * pixRatio,
                             p.y() * pixRatio);
        result.push_back(mode);
    });

    return result;
}

/**
 * @brief Get the list of video modes for a device.
 * @param devName Device name to get nodes from.
 * @return Vector of available modes for the device.
 */
QVector<VideoMode> CameraDevice::getVideoModes(QString devName)
{
    std::ignore = devName;

    if (iformat == nullptr)
        ;
    else if (isScreen(devName))
        return getScreenModes();
#if defined(USING_V4L)
    else if (QString::fromUtf8(iformat->name) == QStringLiteral("video4linux2,v4l2"))
        return v4l2::getDeviceModes(devName);
#endif
#ifdef Q_OS_WIN
    else if (QString::fromUtf8(iformat->name) == QStringLiteral("dshow"))
        return DirectShow::getDeviceModes(devName);
#endif
#ifdef Q_OS_MACOS
    else if (QString::fromUtf8(iformat->name) == QStringLiteral("avfoundation"))
        return avfoundation::getDeviceModes(devName);
#endif
    else
        qWarning() << "Video mode listing not implemented for input" << iformat->name;

    return {};
}

/**
 * @brief Get the name of the pixel format of a video mode.
 * @param pixel_format Pixel format to get the name from.
 * @return Name of the pixel format.
 */
QString CameraDevice::getPixelFormatString(uint32_t pixel_format)
{
#if defined(USING_V4L)
    return v4l2::getPixelFormatString(pixel_format);
#else
    std::ignore = pixel_format;
    return QStringLiteral("unknown");
#endif
}

/**
 * @brief Compare two pixel formats.
 * @param a First pixel format to compare.
 * @param b Second pixel format to compare.
 * @return True if we prefer format a to b,
 * false otherwise (such as if there's no preference).
 */
bool CameraDevice::betterPixelFormat(uint32_t a, uint32_t b)
{
#if defined(USING_V4L)
    return v4l2::betterPixelFormat(a, b);
#else
    std::ignore = a;
    std::ignore = b;
    return false;
#endif
}

/**
 * @brief Sets CameraDevice::iformat to default.
 * @return True if success, false if failure.
 */
bool CameraDevice::getDefaultInputFormat()
{
    const QMutexLocker<QMutex> locker(&iformatLock);
    if (iformat != nullptr) {
        return true;
    }

    avdevice_register_all();

// Desktop capture input formats
#if defined(USING_V4L)
    idesktopFormat = av_find_input_format("x11grab");
#endif
#ifdef Q_OS_WIN
    idesktopFormat = av_find_input_format("gdigrab");
#endif

// Webcam input formats
#if defined(USING_V4L)
    if ((iformat = av_find_input_format("v4l2")) != nullptr)
        return true;
#endif

#ifdef Q_OS_WIN
    if ((iformat = av_find_input_format("dshow")))
        return true;
    if ((iformat = av_find_input_format("vfwcap")))
        return true;
#endif

#ifdef Q_OS_MACOS
    if ((iformat = av_find_input_format("avfoundation")))
        return true;
    if ((iformat = av_find_input_format("qtkit")))
        return true;
#endif

    qWarning() << "No valid input format found";
    return false;
}
