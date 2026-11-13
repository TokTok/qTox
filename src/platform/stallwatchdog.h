/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#pragma once

#include <QTimer>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

/**
 * @brief Logs when the thread that created it stops running its event loop.
 *
 * Silent while the event loop runs. When it hasn't run for the given time, logs
 * a warning with the thread's stack, then a line when it runs again.
 */
class StallWatchdog
{
public:
    explicit StallWatchdog(std::chrono::milliseconds stall);
    ~StallWatchdog();

private:
    void watch();

    const std::chrono::milliseconds stall;
    const std::function<void()> logStack;
    QTimer beat;
    std::atomic<std::chrono::steady_clock::rep> lastBeat;
    std::mutex mutex;
    std::condition_variable wake;
    bool stopping{false};
    std::thread watcher;
};
