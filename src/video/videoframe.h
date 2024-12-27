/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QImage>
#include <QMutex>
#include <QReadWriteLock>
#include <QRect>
#include <QSize>

extern "C"
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <libavcodec/avcodec.h>
#pragma GCC diagnostic pop
}

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

class FrameLocker
{
public:
    enum LockType
    {
        WriteLock,
        ReadLock,
    };

    FrameLocker() = default;

    explicit FrameLocker(QReadWriteLock* lock, LockType type)
        : lock(lock)
    {
        switch (type) {
        case WriteLock:
            lock->lockForWrite();
            break;
        case ReadLock:
            lock->lockForRead();
            break;
        }
    }

    ~FrameLocker()
    {
        if (lock != nullptr) {
            lock->unlock();
        }
    }

    FrameLocker(FrameLocker&& other)
        : lock(other.lock)
    {
        other.lock = nullptr;
    }

    FrameLocker& operator=(FrameLocker&& other)
    {
        if (this != &other) {
            lock = other.lock;
            other.lock = nullptr;
        }

        return *this;
    }

private:
    QReadWriteLock* lock;
};

struct ToxYUVFrame
{
public:
    bool isValid() const;
    explicit operator bool() const;

    const std::uint16_t width;
    const std::uint16_t height;

    const uint8_t* y;
    const uint8_t* u;
    const uint8_t* v;
};

class VideoFrame
{
public:
    // Declare type aliases
    using IDType = std::uint_fast64_t;
    using AtomicIDType = std::atomic_uint_fast64_t;

public:
    VideoFrame(IDType sourceID_, AVFrame* sourceFrame, QRect dimensions, int pixFmt,
               bool freeSourceFrame_ = false);
    VideoFrame(IDType sourceID_, AVFrame* sourceFrame, bool freeSourceFrame_ = false);

    ~VideoFrame();

    // Copy/Move operations are disabled for the VideoFrame, encapsulate with a std::shared_ptr to
    // manage.

    VideoFrame(const VideoFrame& other) = delete;
    VideoFrame(VideoFrame&& other) = delete;

    const VideoFrame& operator=(const VideoFrame& other) = delete;
    const VideoFrame& operator=(VideoFrame&& other) = delete;

    bool isValid();

    std::shared_ptr<VideoFrame> trackFrame();
    static void untrackFrames(const IDType& sourceID, bool releaseFrames = false);

    void releaseFrame();

    std::pair<const AVFrame*, FrameLocker> getAVFrame(QSize frameSize, const int pixelFormat,
                                                      const bool requireAligned);
    QImage toQImage(QSize frameSize = {});
    std::pair<ToxYUVFrame, FrameLocker> toToxYUVFrame(QSize frameSize = {});

    IDType getFrameID() const;
    IDType getSourceID() const;
    QRect getSourceDimensions() const;
    int getSourcePixelFormat() const;

    static constexpr int dataAlignment = 32;

private:
    class FrameBufferKey
    {
    public:
        FrameBufferKey(const int width, const int height, const int pixFmt, const bool lineAligned);

        // Explicitly state default constructor/destructor

        FrameBufferKey(const FrameBufferKey&) = default;
        FrameBufferKey(FrameBufferKey&&) = default;
        ~FrameBufferKey() = default;

        // Assignment operators are disabled for the FrameBufferKey

        const FrameBufferKey& operator=(const FrameBufferKey&) = delete;
        const FrameBufferKey& operator=(FrameBufferKey&&) = delete;

        bool operator==(const FrameBufferKey& other) const;
        bool operator!=(const FrameBufferKey& other) const;

        static size_t hash(const FrameBufferKey& key);

    public:
        const int frameWidth;
        const int frameHeight;
        const int pixelFormat;
        const bool linesizeAligned;
    };

    struct FrameBufferKeyHash
    {
        size_t operator()(const FrameBufferKey& key) const
        {
            return FrameBufferKey::hash(key);
        }
    };

private:
    static FrameBufferKey getFrameKey(const QSize& frameSize, const int pixFmt, const int linesize);
    static FrameBufferKey getFrameKey(const QSize& frameSize, const int pixFmt,
                                      const bool frameAligned);

    AVFrame* retrieveAVFrame(const QSize& dimensions, const int pixelFormat, const bool requireAligned);
    AVFrame* generateAVFrame(const QSize& dimensions, const int pixelFormat, const bool requireAligned);
    AVFrame* storeAVFrame(AVFrame* frame, const QSize& dimensions, const int pixelFormat);

    void deleteFrameBuffer();

    template <typename F>
    std::pair<std::invoke_result_t<F, AVFrame* const>, FrameLocker>
    toGenericObject(const QSize& dimensions, int pixelFormat, bool requireAligned,
                    const F& objectConstructor);

private:
    // ID
    const IDType frameID;
    const IDType sourceID;

    // Main framebuffer store
    std::unordered_map<FrameBufferKey, AVFrame*, FrameBufferKeyHash> frameBuffer{3};

    // Source frame
    const QRect sourceDimensions;
    int sourcePixelFormat;
    const FrameBufferKey sourceFrameKey;
    const bool freeSourceFrame;

    // Reference store
    static AtomicIDType frameIDs;

    static std::unordered_map<IDType, QMutex> mutexMap;
    static std::unordered_map<IDType, std::unordered_map<IDType, std::weak_ptr<VideoFrame>>> refsMap;

    // Concurrency
    QReadWriteLock frameLock{};
    static QReadWriteLock refsLock;
};
