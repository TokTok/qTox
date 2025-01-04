/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <memory>

#include "src/persistence/db/rawdatabase.h"

class RawDatabase;
class IMessageBoxManager;
namespace DbUpgrader {
bool dbSchemaUpgrade(std::shared_ptr<RawDatabase>& db, IMessageBoxManager& messageBoxManager);

bool createCurrentSchema(RawDatabase& db);
bool isNewDb(std::shared_ptr<RawDatabase>& db, bool& success);
bool dbSchema0to1(RawDatabase& db);
bool dbSchema1to2(RawDatabase& db);
bool dbSchema2to3(RawDatabase& db);
bool dbSchema3to4(RawDatabase& db);
bool dbSchema4to5(RawDatabase& db);
bool dbSchema5to6(RawDatabase& db);
bool dbSchema6to7(RawDatabase& db);
bool dbSchema7to8(RawDatabase& db);
bool dbSchema8to9(RawDatabase& db);
bool dbSchema9to10(RawDatabase& db);
// 10to11 from DbTo11::dbSchema10to11

struct BadEntry
{
    BadEntry(int64_t row_, QString toxId_)
        : row{row_}
        , toxId{toxId_}
    {
    }
    RowId row;
    QString toxId;
};
void mergeDuplicatePeers(std::vector<RawDatabase::Query>& upgradeQueries, RawDatabase& db,
                         const std::vector<BadEntry>& badPeers);
} // namespace DbUpgrader
