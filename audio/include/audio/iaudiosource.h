/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QObject>

/**
 * @fn void Audio::frameAvailable(const int16_t *pcm, size_t sample_count, uint8_t channels,
 * uint32_t sampling_rate);
 *
 * When there are input subscribers, we regularly emit captured audio frames with this signal
 * Always connect with a blocking queued connection lambda, else the behaviour is undefined
 */

class IAudioSource : public QObject
{
    Q_OBJECT
public:
    virtual ~IAudioSource() = default;

    virtual operator bool() const = 0;

signals:
    void frameAvailable(const int16_t* pcm, size_t sample_count, uint8_t channels,
                        uint32_t sampling_rate);
    void volumeAvailable(qreal value);
    void invalidated();
};
