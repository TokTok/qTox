/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "browser.h" // IWYU pragma: keep

#ifdef Q_OS_WASM
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

extern "C"
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <libavformat/avformat.h>
#include <libavutil/parseutils.h>
#pragma GCC diagnostic pop

    // See platform/wasm/camera.js.
    void qtox_camera_start(int width, int height);
    int qtox_camera_frame(uint8_t* buf, int size);
    void qtox_camera_stop();
}

namespace {
const QString deviceName = QStringLiteral("browser");
constexpr int ioBufferSize = 64 * 1024;

/**
 * @brief The JPEG image the demuxer is reading.
 */
struct Frame
{
    std::chrono::microseconds interval;
    std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();
    std::vector<uint8_t> data = std::vector<uint8_t>(256 * 1024);
    int size = 0;
    int offset = 0;
};

int readPacket(void* opaque, uint8_t* buf, int bufSize)
{
    auto& frame = *static_cast<Frame*>(opaque);
    if (frame.offset == frame.size) {
        // Take frames at the chosen rate, because the browser has a new one whenever we ask.
        std::this_thread::sleep_until(frame.next);
        frame.next = std::max(frame.next + frame.interval, std::chrono::steady_clock::now());
    }
    while (frame.offset == frame.size) {
        const int size = qtox_camera_frame(frame.data.data(), static_cast<int>(frame.data.size()));
        if (size < 0) {
            return AVERROR(EIO);
        }
        if (static_cast<size_t>(size) > frame.data.size()) {
            frame.data.resize(size);
        } else if (size == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        } else {
            frame.size = size;
            frame.offset = 0;
        }
    }
    const int len = std::min(bufSize, frame.size - frame.offset);
    std::copy_n(frame.data.data() + frame.offset, len, buf);
    frame.offset += len;
    return len;
}

void freeIo(AVIOContext* io)
{
    delete static_cast<Frame*>(io->opaque);
    av_freep(&io->buffer);
    avio_context_free(&io);
    qtox_camera_stop();
}
} // namespace

namespace browser {
QVector<QPair<QString, QString>> getDeviceList()
{
    return {{deviceName, QStringLiteral("Browser camera")}};
}

/**
 * @brief Lists the sizes we ask the browser for.
 *
 * Browsers scale the camera to any size, but we can only send about 6 frames per second at 720p.
 */
QVector<VideoMode> getDeviceModes()
{
    constexpr float fps = 30;
    return {VideoMode{640, 360, 0, 0, fps}, VideoMode{640, 480, 0, 0, fps}};
}

/**
 * @brief Opens the browser's camera as a MJPEG stream, because browsers give us JPEG images.
 */
AVFormatContext* openCamera(AVDictionary** options)
{
    const AVDictionaryEntry* framerate = av_dict_get(*options, "framerate", nullptr, 0);
    const double fps = framerate != nullptr ? std::stod(framerate->value) : 5;
    int width = 0;
    int height = 0;
    const AVDictionaryEntry* videoSize = av_dict_get(*options, "video_size", nullptr, 0);
    if (videoSize != nullptr && av_parse_video_size(&width, &height, videoSize->value) < 0) {
        width = 0;
        height = 0;
    }
    // Take the size out, because the MJPEG demuxer has no such option.
    av_dict_set(options, "video_size", nullptr, 0);

    qtox_camera_start(width, height);
    // Wait for the first frame, so a camera the user didn't allow fails to open.
    int firstFrame = 0;
    while ((firstFrame = qtox_camera_frame(nullptr, 0)) == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
    if (firstFrame < 0) {
        qtox_camera_stop();
        return nullptr;
    }

    auto* const frame = new Frame{std::chrono::microseconds{static_cast<int64_t>(1e6 / fps)}};
    auto* const buffer = static_cast<unsigned char*>(av_malloc(ioBufferSize));
    AVIOContext* io = avio_alloc_context(buffer, ioBufferSize, 0, frame, readPacket, nullptr, nullptr);
    AVFormatContext* context = avformat_alloc_context();
    context->pb = io;
    if (avformat_open_input(&context, nullptr, av_find_input_format("mjpeg"), options) < 0) {
        freeIo(io);
        return nullptr;
    }
    return context;
}

void closeCamera(AVFormatContext** context)
{
    AVIOContext* io = (*context)->pb;
    avformat_close_input(context);
    freeIo(io);
}
} // namespace browser
#endif
