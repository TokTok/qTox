/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

extern "C"
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#pragma GCC diagnostic pop
}
#include "cameradevice.h"
#include "camerasource.h"
#include "videoframe.h"
#include "src/persistence/settings.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QReadLocker>
#include <QWriteLocker>
#include <QtConcurrent/QtConcurrentRun>

/**
 * @class CameraSource
 * @brief This class is a wrapper to share a camera's captured video frames
 *
 * It allows objects to subscribe and unsubscribe to the stream, starting
 * the camera and streaming new video frames only when needed.
 * This is a singleton, since we can only capture from one
 * camera at the same time without thread-safety issues.
 * The source is lazy in the sense that it will only keep the video
 * device open as long as there are subscribers, the source can be
 * open but the device closed if there are zero subscribers.
 */

/**
 * @var QVector<std::weak_ptr<VideoFrame>> CameraSource::freelist
 * @brief Frames that need freeing before we can safely close the device
 *
 * @var QFuture<void> CameraSource::streamFuture
 * @brief Future of the streaming thread
 *
 * @var QString CameraSource::deviceName
 * @brief Short name of the device for CameraDevice's open(QString)
 *
 * @var CameraDevice* CameraSource::device
 * @brief Non-owning pointer to an open CameraDevice, or nullptr. Not atomic, synced
 * with memory fences when becomes null.
 *
 * @var VideoMode CameraSource::mode
 * @brief What mode we tried to open the device in, all zeros means default mode
 *
 * @var AVCodecContext* CameraSource::cctx
 * @brief Codec context of the camera's selected video stream
 *
 * @var AVCodecContext* CameraSource::cctxOrig
 * @brief Codec context of the camera's selected video stream
 * @deprecated
 *
 * @var int CameraSource::videoStreamIndex
 * @brief A camera can have multiple streams, this is the one we're decoding
 *
 * @var QMutex CameraSource::biglock
 * @brief True when locked. Faster than mutexes for video decoding.
 *
 * @var QMutex CameraSource::freelistLock
 * @brief True when locked. Faster than mutexes for video decoding.
 *
 * @var std::atomic_bool CameraSource::streamBlocker
 * @brief Holds the streaming thread still when true
 *
 * @var std::atomic_int CameraSource::subscriptions
 * @brief Remember how many times we subscribed for RAII
 */

namespace {
namespace logcat {
Q_LOGGING_CATEGORY(ffmpeg, "ffmpeg")
Q_LOGGING_CATEGORY(ffmpegInput, "ffmpeg.input")
Q_LOGGING_CATEGORY(ffmpegOutput, "ffmpeg.output")
Q_LOGGING_CATEGORY(ffmpegMuxer, "ffmpeg.muxer")
Q_LOGGING_CATEGORY(ffmpegDemuxer, "ffmpeg.demuxer")
Q_LOGGING_CATEGORY(ffmpegEncoder, "ffmpeg.encoder")
Q_LOGGING_CATEGORY(ffmpegDecoder, "ffmpeg.decoder")
Q_LOGGING_CATEGORY(ffmpegFilter, "ffmpeg.filter")
Q_LOGGING_CATEGORY(ffmpegBitstreamFilter, "ffmpeg.bitstream_filter")
Q_LOGGING_CATEGORY(ffmpegSwscaler, "ffmpeg.swscaler")
Q_LOGGING_CATEGORY(ffmpegSwresampler, "ffmpeg.swresampler")
Q_LOGGING_CATEGORY(ffmpegDeviceVideoOutput, "ffmpeg.device.video_output")
Q_LOGGING_CATEGORY(ffmpegDeviceVideoInput, "ffmpeg.device.video_input")
Q_LOGGING_CATEGORY(ffmpegDeviceAudioOutput, "ffmpeg.device.audio_output")
Q_LOGGING_CATEGORY(ffmpegDeviceAudioInput, "ffmpeg.device.audio_input")
Q_LOGGING_CATEGORY(ffmpegDeviceOutput, "ffmpeg.device.output")
Q_LOGGING_CATEGORY(ffmpegDeviceInput, "ffmpeg.device.input")
} // namespace logcat

const QLoggingCategory& (*avLogCategory(const AVClass* avc))()
{
    if (!avc) {
        return logcat::ffmpeg;
    }

    switch (avc->category) {
    case AV_CLASS_CATEGORY_NA:
    case AV_CLASS_CATEGORY_NB:
        return logcat::ffmpeg;
    case AV_CLASS_CATEGORY_INPUT:
        return logcat::ffmpegInput;
    case AV_CLASS_CATEGORY_OUTPUT:
        return logcat::ffmpegOutput;
    case AV_CLASS_CATEGORY_MUXER:
        return logcat::ffmpegMuxer;
    case AV_CLASS_CATEGORY_DEMUXER:
        return logcat::ffmpegDemuxer;
    case AV_CLASS_CATEGORY_ENCODER:
        return logcat::ffmpegEncoder;
    case AV_CLASS_CATEGORY_DECODER:
        return logcat::ffmpegDecoder;
    case AV_CLASS_CATEGORY_FILTER:
        return logcat::ffmpegFilter;
    case AV_CLASS_CATEGORY_BITSTREAM_FILTER:
        return logcat::ffmpegBitstreamFilter;
    case AV_CLASS_CATEGORY_SWSCALER:
        return logcat::ffmpegSwscaler;
    case AV_CLASS_CATEGORY_SWRESAMPLER:
        return logcat::ffmpegSwresampler;
    case AV_CLASS_CATEGORY_DEVICE_VIDEO_OUTPUT:
        return logcat::ffmpegDeviceVideoOutput;
    case AV_CLASS_CATEGORY_DEVICE_VIDEO_INPUT:
        return logcat::ffmpegDeviceVideoInput;
    case AV_CLASS_CATEGORY_DEVICE_AUDIO_OUTPUT:
        return logcat::ffmpegDeviceAudioOutput;
    case AV_CLASS_CATEGORY_DEVICE_AUDIO_INPUT:
        return logcat::ffmpegDeviceAudioInput;
    case AV_CLASS_CATEGORY_DEVICE_OUTPUT:
        return logcat::ffmpegDeviceOutput;
    case AV_CLASS_CATEGORY_DEVICE_INPUT:
        return logcat::ffmpegDeviceInput;
    }
    return logcat::ffmpeg;
}

const char* avItemName(void* obj, const AVClass* cls)
{
    return (cls->item_name ? cls->item_name : av_default_item_name)(obj);
}

QString avLogType(void* obj, const AVClass* avc)
{
    if (!avc) {
        return QStringLiteral("ffmpeg");
    }

    return QString::fromUtf8(avItemName(obj, avc));
}

Q_ATTRIBUTE_FORMAT_PRINTF(3, 0)
void avLogCallback(void* obj, int level, const char* fmt, va_list args)
{
    const AVClass* avc = obj ? *static_cast<const AVClass* const*>(obj) : nullptr;

    const auto cat = avLogCategory(avc);
    const QString message =
        QStringLiteral("[%1] %3").arg(avLogType(obj, avc)).arg(QString::vasprintf(fmt, args).trimmed());
    switch (level) {
    case AV_LOG_PANIC:
        qFatal("%s", message.toUtf8().constData());
    case AV_LOG_FATAL:
        qCCritical(cat).noquote() << message;
        break;
    case AV_LOG_ERROR:
        qCCritical(cat).noquote() << message;
        break;
    case AV_LOG_WARNING:
        qCWarning(cat).noquote() << message;
        break;
    case AV_LOG_INFO:
        qCInfo(cat).noquote() << message;
        break;
    case AV_LOG_VERBOSE:
        qCDebug(cat).noquote() << message;
        break;
#ifdef QTOX_DEBUG_FFMPEG
    case AV_LOG_DEBUG:
        qCDebug(cat).noquote() << message;
        break;
    case AV_LOG_TRACE:
        qCDebug(cat).noquote() << message;
        break;
    default:
        qCDebug(cat).noquote() << level << message;
        break;
#endif
    }
} // namespace logcat
} // namespace

CameraSource::CameraSource(Settings& settings_)
    : deviceThread{new QThread}
    , deviceName{"none"}
    , device{nullptr}
    , mode{}
    , cctx{nullptr}
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    , cctxOrig{nullptr}
#endif
    , videoStreamIndex{-1}
    , isNone_{true}
    , subscriptions{0}
    , settings{settings_}
{
    av_log_set_callback(avLogCallback);

    qRegisterMetaType<VideoMode>("VideoMode");
    deviceThread->setObjectName("Device thread");
    deviceThread->start();
    moveToThread(deviceThread);

    subscriptions = 0;

// TODO(sudden6): remove code when minimum ffmpeg version >= 4.0
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    av_register_all();
#endif
    avdevice_register_all();
}

/**
 * @brief Setup default device
 * @note If a device is already open, the source will seamlessly switch to the new device.
 */
void CameraSource::setupDefault()
{
    const QString deviceName_ = CameraDevice::getDefaultDeviceName(settings);
    const bool isScreen = CameraDevice::isScreen(deviceName_);
    VideoMode mode_ = VideoMode(settings.getScreenRegion());
    if (!isScreen) {
        mode_ = VideoMode(settings.getCamVideoRes());
        mode_.fps = settings.getCamVideoFPS();
    }

    setupDevice(deviceName_, mode_);
}

/**
 * @brief Change the device and mode.
 * @note If a device is already open, the source will seamlessly switch to the new device.
 */
void CameraSource::setupDevice(const QString& deviceName_, const VideoMode& mode_)
{
    if (QThread::currentThread() != deviceThread) {
        QMetaObject::invokeMethod(this, "setupDevice", Q_ARG(const QString&, deviceName_),
                                  Q_ARG(const VideoMode&, mode_));
        return;
    }

    const QWriteLocker locker{&deviceMutex};

    if (deviceName_ == deviceName && mode_ == mode) {
        return;
    }

    if (subscriptions != 0) {
        // To force close, ignoring optimization
        const int subs = subscriptions;
        subscriptions = 0;
        closeDevice();
        subscriptions = subs;
    }

    deviceName = deviceName_;
    mode = mode_;
    isNone_ = (deviceName == "none");

    if (subscriptions && !isNone_) {
        openDevice();
    }
}

bool CameraSource::isNone() const
{
    return isNone_;
}

CameraSource::~CameraSource()
{
    QWriteLocker locker{&streamMutex};
    const QWriteLocker locker2{&deviceMutex};

    // Stop the device thread
    deviceThread->exit(0);
    deviceThread->wait();
    delete deviceThread;

    if (isNone_) {
        return;
    }

    // Free all remaining VideoFrame
    VideoFrame::untrackFrames(id, true);

    if (cctx) {
        avcodec_free_context(&cctx);
    }
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    if (cctxOrig) {
        avcodec_close(cctxOrig);
    }
#endif

    if (device) {
        for (int i = 0; i < subscriptions; ++i)
            device->close();

        device = nullptr;
    }

    locker.unlock();

    // Synchronize with our stream thread
    while (streamFuture.isRunning())
        QThread::yieldCurrentThread();
}

void CameraSource::subscribe()
{
    const QWriteLocker locker{&deviceMutex};

    ++subscriptions;
    openDevice();
}

void CameraSource::unsubscribe()
{
    const QWriteLocker locker{&deviceMutex};

    --subscriptions;
    if (subscriptions == 0) {
        closeDevice();
    }
}

/**
 * @brief Opens the video device and starts streaming.
 * @note Callers must own the biglock.
 */
void CameraSource::openDevice()
{
    if (QThread::currentThread() != deviceThread) {
        QMetaObject::invokeMethod(this, "openDevice");
        return;
    }

    const QWriteLocker locker{&streamMutex};
    if (subscriptions == 0) {
        return;
    }

    qDebug() << "Opening device" << deviceName << "subscriptions:" << subscriptions;

    if (device) {
        qWarning() << "Device already open";
        device->open();
        emit openFailed();
        return;
    }

    // We need to create a new CameraDevice
    device = CameraDevice::open(deviceName, mode);

    if (!device) {
        qWarning() << "Failed to open device" << deviceName;
        emit openFailed();
        return;
    }

    // We need to open the device as many time as we already have subscribers,
    // otherwise the device could get closed while we still have subscribers
    for (int i = 0; i < subscriptions; ++i) {
        device->open();
    }

    // Find the first video stream, if any
    for (unsigned i = 0; i < device->context->nb_streams; ++i) {
        AVMediaType type;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
        type = device->context->streams[i]->codec->codec_type;
#else
        type = device->context->streams[i]->codecpar->codec_type;
#endif
        if (type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        qWarning() << "Video stream not found";
        emit openFailed();
        return;
    }

    AVCodecID codecId;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    cctxOrig = device->context->streams[videoStreamIndex]->codec;
    codecId = cctxOrig->codec_id;
#else
    // Get the stream's codec's parameters and find a matching decoder
    AVCodecParameters* cparams = device->context->streams[videoStreamIndex]->codecpar;
    codecId = cparams->codec_id;
#endif
    const AVCodec* codec = avcodec_find_decoder(codecId);
    if (!codec) {
        qWarning() << "Codec not found";
        emit openFailed();
        return;
    }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    // Copy context, since we apparently aren't allowed to use the original
    cctx = avcodec_alloc_context3(codec);
    if (avcodec_copy_context(cctx, cctxOrig) != 0) {
        qWarning() << "Can't copy context";
        emit openFailed();
        return;
    }

    cctx->refcounted_frames = 1;
#else
    // Create a context for our codec, using the existing parameters
    cctx = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(cctx, cparams) < 0) {
        qWarning() << "Can't create AV context from parameters";
        emit openFailed();
        return;
    }
#endif

    // Open codec
    if (avcodec_open2(cctx, codec, nullptr) < 0) {
        qWarning() << "Can't open codec" << codec->name;
        avcodec_free_context(&cctx);
        emit openFailed();
        return;
    }

    if (streamFuture.isRunning()) {
        qDebug() << "The stream thread is already running! Keeping the current one open.";
    } else {
        streamFuture = QtConcurrent::run([this] { stream(); });
    }

    // Synchronize with our stream thread
    while (!streamFuture.isRunning())
        QThread::yieldCurrentThread();

    qDebug() << "Opened device" << deviceName << "with codec" << codec->name;
    emit deviceOpened();
}

/**
 * @brief Closes the video device and stops streaming.
 * @note Callers must own the biglock.
 */
void CameraSource::closeDevice()
{
    if (QThread::currentThread() != deviceThread) {
        QMetaObject::invokeMethod(this, "closeDevice");
        return;
    }

    const QWriteLocker locker{&streamMutex};
    if (subscriptions != 0) {
        return;
    }

    qDebug() << "Closing device" << deviceName << "subscriptions:" << subscriptions;

    // Free all remaining VideoFrame
    VideoFrame::untrackFrames(id, true);

    // Free our resources and close the device
    videoStreamIndex = -1;
    avcodec_free_context(&cctx);
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
    avcodec_close(cctxOrig);
    cctxOrig = nullptr;
#endif
    while (device && !device->close()) {
    }
    device = nullptr;
}

/**
 * @brief Blocking. Decodes video stream and emits new frames.
 * @note Designed to run in its own thread.
 */
void CameraSource::stream()
{
    auto streamLoop = [this]() {
        AVPacket packet;
        if (av_read_frame(device->context, &packet) != 0) {
            return;
        }

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(57, 48, 101)
        AVFrame* frame = av_frame_alloc();
        if (!frame) {
            return;
        }

        // Only keep packets from the right stream;
        if (packet.stream_index == videoStreamIndex) {
            // Decode video frame
            int frameFinished;
            avcodec_decode_video2(cctx, frame, &frameFinished, &packet);
            if (!frameFinished) {
                return;
            }

            emit frameAvailable(VideoFrame::fromAVFrame(id, frame));
        }
#else

        // Forward packets to the decoder and grab the decoded frame
        const bool isVideo = packet.stream_index == videoStreamIndex;
        const bool readyToReceive = isVideo && !avcodec_send_packet(cctx, &packet);

        if (readyToReceive) {
            AVFrame* frame = av_frame_alloc();
            if (frame && !avcodec_receive_frame(cctx, frame)) {
                emit frameAvailable(VideoFrame::fromAVFrame(id, frame));
            } else {
                av_frame_free(&frame);
            }
        }
#endif

        av_packet_unref(&packet);
    };

    forever
    {
        const QReadLocker locker{&streamMutex};

        // Exit if device is no longer valid
        if (!device) {
            break;
        }

        streamLoop();
    }
}
