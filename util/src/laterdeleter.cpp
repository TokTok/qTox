/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#include "util/laterdeleter.h"

#include <QObject>

LaterDeleter::LaterDeleter(QObject* ptr_)
    : ptr(ptr_)
{
}

LaterDeleter::~LaterDeleter()
{
    ptr->deleteLater();
}
