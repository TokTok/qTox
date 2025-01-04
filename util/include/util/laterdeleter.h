/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#pragma once

class QObject;

struct LaterDeleter
{
    QObject* ptr;

    explicit LaterDeleter(QObject* ptr_);
    ~LaterDeleter();
};
