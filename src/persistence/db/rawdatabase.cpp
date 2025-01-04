/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "rawdatabase.h"

#include "rawdatabaseimpl.h"

RawDatabase::~RawDatabase() = default;

std::shared_ptr<RawDatabase> RawDatabase::open(const QString& path, const QString& password,
                                               const QByteArray& salt)
{
    return std::make_shared<RawDatabaseImpl>(path, password, salt);
}
