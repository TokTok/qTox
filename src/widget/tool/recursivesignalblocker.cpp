/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "recursivesignalblocker.h"

#include <QObject>
#include <QSignalBlocker>

/**
@class  RecursiveSignalBlocker
@brief  Recursively blocks all signals from an object and its children.
@note   All children must be created before the blocker is used.

Wraps a QSignalBlocker on each object. Signals will be unblocked when the
blocker gets destroyed. According to QSignalBlocker, we are also exception safe.
*/

/**
@brief      Creates a QSignalBlocker recursively on the object and child objects.
@param[in]  object  the object, which signals should be blocked
*/
RecursiveSignalBlocker::RecursiveSignalBlocker(QObject* object)
{
    recursiveBlock(object);
}

RecursiveSignalBlocker::~RecursiveSignalBlocker()
{
    qDeleteAll(mBlockers);
}

/**
@brief      Recursively blocks all signals of the object.
@param[in]  object  the object to block
*/
void RecursiveSignalBlocker::recursiveBlock(QObject* object)
{
    mBlockers << new QSignalBlocker(object);

    for (QObject* child : object->children()) {
        recursiveBlock(child);
    }
}
