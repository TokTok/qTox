/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QObject>

#include <atomic>
#include <memory>

class VideoFrame;

class VideoSource : public QObject
{
    Q_OBJECT

public:
    // Declare type aliases
    using IDType = std::uint_fast64_t;
    using AtomicIDType = std::atomic_uint_fast64_t;

public:
    VideoSource()
        : id(sourceIDs++)
    {
    }

    virtual ~VideoSource() = default;
    /**
     * @brief If subscribe successfully opens the source, it will start emitting frameAvailable
     * signals.
     */
    virtual void subscribe() = 0;
    /**
     * @brief Stop emitting frameAvailable signals, and free associated resources if necessary.
     */
    virtual void unsubscribe() = 0;

    /// ID of this VideoSource
    const IDType id;
signals:
    /**
     * @brief Emitted when new frame available to use.
     * @param frame New frame.
     */
    void frameAvailable(std::shared_ptr<VideoFrame> frame);
    /**
     * @brief Emitted when the source is stopped for an indefinite amount of time, but might restart
     * sending frames again later
     */
    void sourceStopped();

private:
    // Used to manage a global ID for all VideoSources
    static AtomicIDType sourceIDs;
};
