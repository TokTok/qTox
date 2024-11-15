/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QString>

#include <array>
#include <memory>
#include <vector>

class RawDatabase;

namespace DbUtility
{
    struct SqliteMasterEntry {
        QString name;
        QString sql;
        bool operator==(const DbUtility::SqliteMasterEntry& rhs) const;
    };

    extern const std::array<QString, 11> testFileList;
    extern const std::vector<SqliteMasterEntry> schema0;
    extern const std::vector<SqliteMasterEntry> schema1;
    extern const std::vector<SqliteMasterEntry> schema2;
    extern const std::vector<SqliteMasterEntry> schema3;
    extern const std::vector<SqliteMasterEntry> schema4;
    extern const std::vector<SqliteMasterEntry> schema5;
    extern const std::vector<SqliteMasterEntry> schema6;
    extern const std::vector<SqliteMasterEntry> schema7;
    extern const std::vector<SqliteMasterEntry> schema9;
    extern const std::vector<SqliteMasterEntry> schema10;
    extern const std::vector<SqliteMasterEntry> schema11;

    void createSchemaAtVersion(std::shared_ptr<RawDatabase> db, const std::vector<DbUtility::SqliteMasterEntry>& schema);
    void verifyDb(std::shared_ptr<RawDatabase> db, const std::vector<DbUtility::SqliteMasterEntry>& expectedSql);
}
