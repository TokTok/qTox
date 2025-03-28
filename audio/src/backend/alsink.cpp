/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "audio/src/backend/alsink.h"

#include "audio/src/backend/openal.h"

#include <QDebug>
#include <QMutexLocker>

/**
 * @brief Can play audio via the speakers or some other audio device. Allocates
 *        and frees the audio ressources internally.
 */

AlSink::~AlSink()
{
    const QMutexLocker<QRecursiveMutex> locker{&killLock};

    // unsubscribe only if not already killed
    if (!killed) {
        audio.destroySink(*this);
        killed = true;
    }
}

void AlSink::playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const
{
    const QMutexLocker<QRecursiveMutex> locker{&killLock};

    if (killed) {
        qCritical() << "Trying to play audio on an invalid sink";
    } else {
        audio.playAudioBuffer(sourceId, data, samples, channels, sampleRate);
    }
}

void AlSink::playMono16Sound(const IAudioSink::Sound& sound)
{
    const QMutexLocker<QRecursiveMutex> locker{&killLock};

    if (killed) {
        qCritical() << "Trying to play sound on an invalid sink";
    } else {
        audio.playMono16Sound(*this, sound);
    }
}

void AlSink::startLoop()
{
    const QMutexLocker<QRecursiveMutex> locker{&killLock};

    if (killed) {
        qCritical() << "Trying to start loop on an invalid sink";
    } else {
        audio.startLoop(sourceId);
    }
}

void AlSink::stopLoop()
{
    const QMutexLocker<QRecursiveMutex> locker{&killLock};

    if (killed) {
        qCritical() << "Trying to stop loop on an invalid sink";
    } else {
        audio.stopLoop(sourceId);
    }
}

uint AlSink::getSourceId() const
{
    const uint tmp = sourceId;
    return tmp;
}

void AlSink::kill()
{
    killLock.lock();
    // this flag is only set once here, afterwards the object is considered dead
    killed = true;
    killLock.unlock();
    emit invalidated();
}

AlSink::AlSink(OpenAL& al, uint sourceId_)
    : audio(al)
    , sourceId{sourceId_}
{
}

AlSink::operator bool() const
{
    const QMutexLocker<QRecursiveMutex> locker{&killLock};

    return !killed;
}
