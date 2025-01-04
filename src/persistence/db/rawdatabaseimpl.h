/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "rawdatabase.h"

#include <QByteArray>
#include <QMutex>
#include <QQueue>
#include <QRegularExpression>
#include <QString>
#include <QThread>
#include <QVariant>

#include <queue>
#include <atomic>
#include <memory>
#include <vector>

/// The two following defines are required to use SQLCipher
/// They are used by the sqlite3.h header
#define SQLITE_HAS_CODEC
#define SQLITE_TEMP_STORE 2

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <sqlite3.h>
#pragma GCC diagnostic pop

class RawDatabaseImpl final : QObject, public RawDatabase
{
    Q_OBJECT

private:
    static void regexp(sqlite3_context* ctx, int argc, sqlite3_value** argv,
                       const QRegularExpression::PatternOptions cs);

    /**
     * @struct Transaction
     * @brief SQL transactions to be processed.
     *
     * A transaction is made of queries, which can have bound BLOBs.
     *
     * @var std::atomic_bool* RawDatabaseImpl::Transaction::success = nullptr;
     * @brief If not a nullptr, the result of the transaction will be set
     *
     * @var std::atomic_bool* RawDatabaseImpl::Transaction::done = nullptr;
     * @brief If not a nullptr, will be set to true when the transaction has been executed
     */
    struct Transaction
    {
        std::vector<Query> queries;
        std::atomic_bool* success = nullptr;
        std::atomic_bool* done = nullptr;
    };

public:
    enum class SqlCipherParams
    {
        // keep these sorted in upgrade order
        p3_0, // SQLCipher 3.0 default encryption params
              // SQLCipher 4.0 default params where SQLCipher 3.0 supports them, but 3.0 params
              // where not possible. We accidentally got to this state when attemption to update all
              // databases to 4.0 defaults even when using SQLCipher 3.x, but might as well keep
              // using these for people with SQLCipher 3.x.
        halfUpgradedTo4,
        p4_0 // SQLCipher 4.0 default encryption params
    };

    RawDatabaseImpl(const QString& path_, const QString& password, const QByteArray& salt);
    ~RawDatabaseImpl() override;
    bool isOpen() override;

    bool execNow(const QString& statement) override;
    bool execNow(Query statement) override;
    bool execNow(std::vector<Query> statements) override;

    void execLater(const QString& statement) override;
    void execLater(Query statement) override;
    void execLater(std::vector<Query> statements) override;

    void sync() override;

    static QString toString(SqlCipherParams params)
    {
        switch (params) {
        case SqlCipherParams::p3_0:
            return "3.0 default";
        case SqlCipherParams::halfUpgradedTo4:
            return "3.x max compatible";
        case SqlCipherParams::p4_0:
            return "4.0 default";
        }
        qFatal("Invalid SqlCipherParams");
    }

public slots:
    bool setPassword(const QString& password) override;
    bool rename(const QString& newPath) override;
    bool remove() override;

protected slots:
    bool open(const QString& path_, const QString& hexKey = {});
    void close();
    void process();

private:
    void compileAndExecute(Transaction& trans);
    QString anonymizeQuery(const QByteArray& query);
    bool openEncryptedDatabaseAtLatestSupportedVersion(const QString& hexKey);
    bool updateSavedCipherParameters(const QString& hexKey, SqlCipherParams newParams);
    bool setCipherParameters(SqlCipherParams params, const QString& database = {});
    SqlCipherParams highestSupportedParams();
    SqlCipherParams readSavedCipherParams(const QString& hexKey, SqlCipherParams newParams);
    bool setKey(const QString& hexKey);
    int getUserVersion();
    bool encryptDatabase(const QString& newHexKey);
    bool decryptDatabase();
    bool commitDbSwap(const QString& hexKey);
    bool testUsable();

protected:
    static QString deriveKey(const QString& password, const QByteArray& salt);
    static QString deriveKey(const QString& password);
    static QVariant extractData(sqlite3_stmt* stmt, int col);
    static void regexpInsensitive(sqlite3_context* ctx, int argc, sqlite3_value** argv);
    static void regexpSensitive(sqlite3_context* ctx, int argc, sqlite3_value** argv);

private:
    sqlite3* sqlite;
    std::unique_ptr<QThread> workerThread;
    std::queue<Transaction> pendingTransactions;
    QMutex transactionsMutex;
    QString path;
    QByteArray currentSalt;
    QString currentHexKey;
};
