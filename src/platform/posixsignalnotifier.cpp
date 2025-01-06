/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "posixsignalnotifier.h"

#ifndef Q_OS_WIN
#include <QDebug>
#include <QSocketNotifier>

#include <array>
#include <atomic>

#include <errno.h>
#include <signal.h>     // sigaction()
#include <sys/socket.h> // socketpair()
#include <sys/types.h>  // may be needed for BSD
#include <unistd.h>     // close()

/**
 * @class PosixSignalNotifier
 * @brief Class for converting POSIX signals to Qt signals
 */

namespace {

std::atomic_flag g_signalSocketUsageFlag = ATOMIC_FLAG_INIT;
std::array<int, 2> g_signalSocketPair;

void signalHandler(int signum)
{
    // DO NOT call any Qt functions directly, only limited amount of so-called async-signal-safe
    // functions can be called in signal handlers.
    // See https://doc.qt.io/qt-4.8/unix-signals.html

    // If test_and_set() returns true, it means it was already in use (only by
    // ~PosixSignalNotifier()), so we bail out. Our signal handler is blocking, only one will be
    // called (no race between threads), hence simple implementation.
    if (g_signalSocketUsageFlag.test_and_set())
        return;

    if (::write(g_signalSocketPair[0], &signum, sizeof(signum)) == -1) {
        // We hardly can do anything more useful in signal handler, and
        // any ways it's probably very unexpected error (out of memory?),
        // since we check socket existence with a flag beforehand.
        abort();
    }

    g_signalSocketUsageFlag.clear();
}

constexpr std::initializer_list<int> terminatingSignals{SIGHUP, SIGINT, SIGQUIT, SIGTERM};
constexpr std::initializer_list<int> usrSignals{SIGUSR1, SIGUSR2};

} // namespace

PosixSignalNotifier::~PosixSignalNotifier()
{
    while (g_signalSocketUsageFlag.test_and_set()) {
        // spin-loop until we acquire flag (signal handler might be running and have flag in use)
    }

    // do not leak sockets
    ::close(g_signalSocketPair[0]);
    ::close(g_signalSocketPair[1]);

    // do not clear the usage flag here, signal handler cannot use socket any more!
}

void PosixSignalNotifier::watchSignal(int signum)
{
    sigset_t blockMask;
    sigemptyset(&blockMask);       // do not prefix with ::, it's a macro on macOS
    sigaddset(&blockMask, signum); // do not prefix with ::, it's a macro on macOS

    struct sigaction action = {}; // all zeroes by default
    action.sa_handler = signalHandler;
    action.sa_mask = blockMask; // allow old signal to finish before new is raised

    if (::sigaction(signum, &action, nullptr)) {
        qFatal("Failed to setup signal %d, error = %d", signum, errno);
    }
}

void PosixSignalNotifier::watchSignals(std::initializer_list<int> signalSet)
{
    for (auto s : signalSet) {
        watchSignal(s);
    }
}

void PosixSignalNotifier::watchCommonTerminatingSignals()
{
    watchSignals(terminatingSignals);
}

void PosixSignalNotifier::watchUsrSignals()
{
    watchSignals(usrSignals);
}

void PosixSignalNotifier::unwatchSignal(int signum)
{
    struct sigaction action = {}; // all zeroes by default
    action.sa_handler = [](int sig) {
        qWarning("Signal %d received twice; terminating ungracefully", sig);
        ::exit(EXIT_FAILURE);
    };

    if (::sigaction(signum, &action, nullptr)) {
        qFatal("Failed to reset signal %d, error = %d", signum, errno);
    }
}

void PosixSignalNotifier::onSignalReceived()
{
    int signum{0};
    if (::read(g_signalSocketPair[1], &signum, sizeof(signum)) == -1) {
        qFatal("Failed to read from signal socket, error = %d", errno);
    }

    switch (signum) {
    case SIGUSR1:
        emit usrSignal(UserSignal::Screenshot);
        break;
    case SIGUSR2:
        emit usrSignal(UserSignal::Unused);
        break;
    default:
        qDebug() << "Terminating signal" << signum << "received";
        emit terminatingSignal(signum);
        unwatchSignal(signum);
        break;
    }
}

PosixSignalNotifier::PosixSignalNotifier()
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, g_signalSocketPair.data())) {
        qFatal("Failed to create socket pair, error = %d", errno);
    }

    notifier = new QSocketNotifier(g_signalSocketPair[1], QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &PosixSignalNotifier::onSignalReceived);
}
#else  // Q_OS_WIN
PosixSignalNotifier::~PosixSignalNotifier() = default;

void PosixSignalNotifier::watchSignal(int signum)
{
    std::ignore = signum;
}

void PosixSignalNotifier::watchSignals(std::initializer_list<int> signalSet)
{
    std::ignore = signalSet;
}

void PosixSignalNotifier::watchCommonTerminatingSignals() {}

void PosixSignalNotifier::watchUsrSignals() {}

void PosixSignalNotifier::onSignalReceived() {}

PosixSignalNotifier::PosixSignalNotifier() {}
#endif // Q_OS_WIN

PosixSignalNotifier& PosixSignalNotifier::globalInstance()
{
    static PosixSignalNotifier instance;
    return instance;
}
