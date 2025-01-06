/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QObject>

class QSocketNotifier;

class PosixSignalNotifier : public QObject
{
    Q_OBJECT

public:
    enum class UserSignal
    {
        Invalid = 0,
        Screenshot,
        Unused,
    };
    Q_ENUM(UserSignal)

    ~PosixSignalNotifier();

    static void watchSignal(int signum);
    static void watchSignals(std::initializer_list<int> signalSet);
    static void watchCommonTerminatingSignals();
    static void watchUsrSignals();

    static void unwatchSignal(int signum);

    static PosixSignalNotifier& globalInstance();

signals:
    void terminatingSignal(int signal);
    void usrSignal(UserSignal signal);

private slots:
    void onSignalReceived();

private:
    PosixSignalNotifier();

private:
    QSocketNotifier* notifier{nullptr};
};
