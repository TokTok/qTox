/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2018-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QTime>

#include <array>

class ToxFileProgress
{
public:
    explicit ToxFileProgress(uint64_t filesize_, int samplePeriodMs_ = 4000);

    QTime lastSampleTime() const;
    bool addSample(uint64_t bytesSent, QTime now = QTime::currentTime());
    void resetSpeed();
    void resetTo(uint64_t bytesSent);

    uint64_t getBytesSent() const;
    uint64_t getFileSize() const
    {
        return filesize;
    }
    double getProgress() const;
    double getSpeed() const;
    double getTimeLeftSeconds() const;

private:
    // Should never be modified, but do not want to lose assignment operators
    uint64_t filesize;
    int samplePeriodMs;

    struct Sample
    {
        uint64_t bytesSent = 0;
        QTime timestamp;
    };

    std::array<Sample, 2> samples;
    uint8_t activeSample = 0;
};
