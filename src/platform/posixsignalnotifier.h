/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QObject>

#ifndef Q_OS_WIN
class QSocketNotifier;

class PosixSignalNotifier : public QObject
{
    Q_OBJECT

public:
    ~PosixSignalNotifier();

    static void watchSignal(int signum);
    static void watchSignals(std::initializer_list<int> signalSet);
    static void watchCommonTerminatingSignals();

    static PosixSignalNotifier& globalInstance();

signals:
    void activated(int signal);

private slots:
    void onSignalReceived();

private:
    PosixSignalNotifier();

private:
    QSocketNotifier* notifier{nullptr};
};
#endif
