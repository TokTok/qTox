/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/persistence/db/rawdatabase.h"
#include "src/persistence/db/upgrades/dbupgrader.h"

#include <vector>

class ToxPk;

namespace DbTo11 {
bool dbSchema10to11(RawDatabase& db);
bool appendDeduplicatePeersQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);
bool getInvalidPeers(RawDatabase& db, std::vector<DbUpgrader::BadEntry>& badPeers);
bool appendSplitPeersQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);


namespace PeersToAuthors {
bool appendPeersToAuthorsQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);
void appendCreateNewTablesQueries(std::vector<RawDatabase::Query>& upgradeQueries);
bool appendPopulateAuthorQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);
bool appendUpdateAliasesFkQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);
void appendReplaceOldTablesQueries(std::vector<RawDatabase::Query>& upgradeQueries);
} // namespace PeersToAuthors

namespace PeersToChats {
bool appendPeersToChatsQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);
void appendCreateNewTablesQueries(std::vector<RawDatabase::Query>& upgradeQueries);
bool appendPopulateChatsQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);
bool appendUpdateHistoryFkQueries(RawDatabase& db, std::vector<RawDatabase::Query>& upgradeQueries);
void appendReplaceOldTablesQueries(std::vector<RawDatabase::Query>& upgradeQueries);
void appendUpdateSystemMessagesFkQueries(std::vector<RawDatabase::Query>& upgradeQueries);
void appendUpdateBrokenMessagesFkQueries(std::vector<RawDatabase::Query>& upgradeQueries);
void appendUpdateFauxOfflinePendingFkQueries(std::vector<RawDatabase::Query>& upgradeQueries);
} // namespace PeersToChats

void appendUpdateTextMessagesFkQueries(std::vector<RawDatabase::Query>& upgradeQueries);
void appendUpdateFileTransfersFkQueries(std::vector<RawDatabase::Query>& upgradeQueries);
void appendDropPeersQueries(std::vector<RawDatabase::Query>& upgradeQueries);
} // namespace DbTo11
