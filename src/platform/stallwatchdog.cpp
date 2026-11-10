/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "stallwatchdog.h"

#include <QDebug>

#include <optional>
#include <tuple>
#include <utility>

#if !defined(Q_OS_WIN) && !defined(Q_OS_WASM)
#include "src/platform/stacktrace.h"

#include <array>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <pthread.h>
#endif

using Clock = std::chrono::steady_clock;

namespace {
// Beats and checks per stall, so a running event loop stays well under it, and
// a stall is logged soon after it reaches it.
constexpr int checksPerStall = 5;

#if !defined(Q_OS_WIN) && !defined(Q_OS_WASM)
// Ignored by default and unused by qTox and Qt, so a stray one does no harm.
constexpr int stackSignal = SIGURG;

using Name = std::array<char, 256>;

struct CapturedFrame
{
    uint64_t base;
    uint64_t offset;
    Name object;
    Name function;
};

std::array<CapturedFrame, 64> g_frames;
std::atomic<int> g_frameCount{0};
std::atomic<bool> g_wanted{false};
std::atomic<bool> g_captured{false};

void copyName(Name& to, const char* from)
{
    std::strncpy(to.data(), from != nullptr ? from : "", to.size() - 1);
    to.back() = '\0';
}

// Runs on the stalled thread: no heap, only copies into the frames above.
void captureStack(int signum)
{
    std::ignore = signum;
    if (!g_wanted.exchange(false)) {
        return;
    }

    int count = 0;
    Stacktrace::process([&count](const Stacktrace::Frame& frame) {
        if (std::cmp_equal(count, g_frames.size())) {
            return;
        }
        CapturedFrame& captured = g_frames[count++];
        captured.base = frame.base;
        captured.offset = frame.offset;
        copyName(captured.object, frame.object);
        copyName(captured.function, frame.function);
    });
    g_frameCount = count;
    g_captured = true;
}

std::function<void()> stackLogger()
{
    // The unwinder loads what it needs on first use: here, not in the handler.
    Stacktrace::process([](const Stacktrace::Frame& frame) { std::ignore = frame; });

    struct sigaction action = {};
    sigemptyset(&action.sa_mask);
    action.sa_handler = captureStack;
    // Restart the system calls it interrupts in the stalled thread.
    action.sa_flags = SA_RESTART;
    if (::sigaction(stackSignal, &action, nullptr) != 0) {
        qWarning() << "Stall watchdog can't log stacks, sigaction error =" << errno;
        return [] {};
    }

    const pthread_t thread = pthread_self();
    return [thread] {
        g_captured = false;
        g_wanted = true;
        if (pthread_kill(thread, stackSignal) != 0) {
            g_wanted = false;
            return;
        }

        const auto deadline = Clock::now() + std::chrono::seconds(1);
        while (!g_captured && Clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (!g_captured) {
            g_wanted = false;
            qWarning() << "    (no stack: the signal handler didn't run)";
            return;
        }

        for (int i = 0; i < g_frameCount; ++i) {
            const CapturedFrame& frame = g_frames[i];
            const char* name = frame.function[0] != '\0' ? frame.function.data() : nullptr;
            qWarning() << Stacktrace::Frame{i, frame.base, frame.object.data(), name, frame.offset};
        }
    };
}
#else
std::function<void()> stackLogger()
{
    return [] {};
}
#endif

Clock::rep ticks(Clock::time_point time)
{
    return time.time_since_epoch().count();
}

qint64 millis(Clock::duration duration)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
} // namespace

StallWatchdog::StallWatchdog(std::chrono::milliseconds stall_)
    : stall{stall_}
    , logStack{stackLogger()}
    , lastBeat{ticks(Clock::now())}
{
    beat.setInterval(stall / checksPerStall);
    QObject::connect(&beat, &QTimer::timeout, &beat, [this] { lastBeat = ticks(Clock::now()); });
    beat.start();
    watcher = std::thread([this] { watch(); });
}

StallWatchdog::~StallWatchdog()
{
    {
        const std::scoped_lock<std::mutex> lock(mutex);
        stopping = true;
    }
    wake.notify_one();
    watcher.join();
}

void StallWatchdog::watch()
{
    const auto tick = stall / checksPerStall;
    auto lastTick = Clock::now();
    std::optional<Clock::time_point> stalledSince;

    std::unique_lock<std::mutex> lock(mutex);
    while (!wake.wait_for(lock, tick, [this] { return stopping; })) {
        const auto now = Clock::now();
        const Clock::time_point beatAt{Clock::duration{lastBeat.load()}};
        // We didn't run either: the whole process was paused (suspend, debugger).
        const bool paused = now - lastTick >= stall;
        lastTick = now;

        if (stalledSince) {
            if (beatAt > *stalledSince) {
                qWarning().nospace()
                    << "Event loop ran again after " << millis(beatAt - *stalledSince) << " ms";
                stalledSince.reset();
            }
        } else if (!paused && now - beatAt >= stall) {
            stalledSince = beatAt;
            qWarning().nospace() << "Event loop stalled for " << millis(now - beatAt) << " ms, in:";
            logStack();
        }
    }
}
