/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "posixsignalnotifier.h"

#ifndef Q_OS_WIN
#include "src/platform/stacktrace.h"

#include <QDebug>
#include <QSocketNotifier>

#include <array>
#include <atomic>

#include <cerrno>
#include <csignal>      // sigaction()
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
constexpr std::initializer_list<int> crashSignals{SIGSEGV, SIGILL, SIGFPE, SIGABRT, SIGBUS, SIGSYS};

// Give it 128 KiB, should be enough for what we're doing in the crash handler.
static uint8_t alternate_stack[128 * 1024];

void installCrashHandler()
{
    qDebug() << "Installing crash handler";

    stack_t ss = {};
    ss.ss_sp = alternate_stack;
    ss.ss_size = sizeof(alternate_stack);
    ss.ss_flags = 0;

    if (sigaltstack(&ss, nullptr) != 0) {
        qFatal("sigaltstack failed");
    }

    struct sigaction action = {};
    sigemptyset(&action.sa_mask);

#ifdef Q_OS_MACOS
    /* for some reason backtrace() doesn't work on macOS
       when we use an alternate stack */
    action.sa_flags = SA_SIGINFO;
#else
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
#endif

    action.sa_handler = [](int sig) {
        // Uninstall the signal handler to avoid infinite recursion.
        constexpr struct sigaction default_action = {};
        ::sigaction(sig, &default_action, nullptr);

        // Print to qCritical, which will write to the log file, if possible.
        // This might crash more or fail allocations. It's best effort.
        qCritical("Crash signal %d received", sig);
        Stacktrace::process([](const Stacktrace::Frame& frame) { qCritical() << frame; });

        // We let the handler return to trigger the default action.
    };

    for (auto s : crashSignals) {
        if (::sigaction(s, &action, nullptr) != 0) {
            qFatal("Failed to setup crash signal %d, error = %d", s, errno);
        }
    }
}

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

    if (::sigaction(signum, &action, nullptr) != 0) {
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

    if (::sigaction(signum, &action, nullptr) != 0) {
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
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, g_signalSocketPair.data()) != 0) {
        qFatal("Failed to create socket pair, error = %d", errno);
    }

    notifier = new QSocketNotifier(g_signalSocketPair[1], QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &PosixSignalNotifier::onSignalReceived);

    installCrashHandler();
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
