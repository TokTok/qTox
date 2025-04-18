/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "audio/iaudiosink.h"
#include "util/interface.h"

#include <QMutex>
#include <QObject>

class OpenAL;
class QMutex;
class AlSink : public QObject, public IAudioSink
{
    Q_OBJECT
public:
    AlSink(OpenAL& al, uint sourceId);
    AlSink(const AlSink& src) = delete;
    AlSink& operator=(const AlSink&) = delete;
    AlSink(AlSink&& other) = delete;
    AlSink& operator=(AlSink&& other) = delete;
    ~AlSink() override;

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels,
                         int sampleRate) const override;
    void playMono16Sound(const IAudioSink::Sound& sound) override;
    void startLoop() override;
    void stopLoop() override;

    explicit operator bool() const override;

    uint getSourceId() const;
    void kill();

    SIGNAL_IMPL(AlSink, finishedPlaying, void)
    SIGNAL_IMPL(AlSink, invalidated, void)

private:
    OpenAL& audio;
    uint sourceId;
    bool killed = false;
    mutable QRecursiveMutex killLock;
};
