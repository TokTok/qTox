/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QVector>

class QObject;
class QSignalBlocker;

class RecursiveSignalBlocker
{
public:
    explicit RecursiveSignalBlocker(QObject* object);
    ~RecursiveSignalBlocker();

    void recursiveBlock(QObject* object);

private:
    QVector<const QSignalBlocker*> mBlockers;
};
