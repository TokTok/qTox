/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "audio/iaudiosource.h"
#include "util/compatiblerecursivemutex.h"
#include <QMutex>
#include <QObject>

class OpenAL;
class AlSource : public IAudioSource
{
    Q_OBJECT
public:
    AlSource(OpenAL& al);
    AlSource(AlSource& src) = delete;
    AlSource& operator=(const AlSource&) = delete;
    AlSource(AlSource&& other) = delete;
    AlSource& operator=(AlSource&& other) = delete;
    ~AlSource();

    operator bool() const;

    void kill();

private:
    OpenAL& audio;
    bool killed = false;
    mutable CompatibleRecursiveMutex killLock;
};
