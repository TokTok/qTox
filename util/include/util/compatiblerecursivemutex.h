/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2021 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QtGlobal>
#include <QMutex>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
class CompatibleRecursiveMutex : public QRecursiveMutex
{};
#else
class CompatibleRecursiveMutex : public QMutex
{
public:
    CompatibleRecursiveMutex()
        : QMutex(QMutex::Recursive)
    {}
};
#endif
