/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "audio/src/backend/alsource.h"
#include "audio/src/backend/openal.h"

/**
 * @brief Emits audio frames captured by an input device or other audio source.
 */

/**
 * @brief Reserves ressources for an audio source
 * @param audio Main audio object, must have longer lifetime than this object.
 */
AlSource::AlSource(OpenAL& al)
    : audio(al)
{}

AlSource::~AlSource()
{
    QMutexLocker<CompatibleRecursiveMutex> locker{&killLock};

    // unsubscribe only if not already killed
    if (!killed) {
        audio.destroySource(*this);
        killed = true;
    }
}

AlSource::operator bool() const
{
    QMutexLocker<CompatibleRecursiveMutex> locker{&killLock};
    return !killed;
}

void AlSource::kill()
{
    killLock.lock();
    // this flag is only set once here, afterwards the object is considered dead
    killed = true;
    killLock.unlock();
    emit invalidated();
}
