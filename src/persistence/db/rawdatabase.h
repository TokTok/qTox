/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "util/strongtype.h"

#include <QByteArray>
#include <QMutex>
#include <QPair>
#include <QQueue>
#include <QRegularExpression>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QVector>

#include <atomic>
#include <cassert>
#include <functional>
#include <memory>

/// The two following defines are required to use SQLCipher
/// They are used by the sqlite3.h header
#define SQLITE_HAS_CODEC
#define SQLITE_TEMP_STORE 2

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <sqlite3.h>
#pragma GCC diagnostic pop

using RowId = NamedType<int64_t, struct RowIdTag, Orderable>;
Q_DECLARE_METATYPE(RowId)

class RawDatabase : QObject
{
    Q_OBJECT

public:
    class Query
    {
    public:
        Query(QString query_, QVector<QByteArray> blobs_ = {},
              const std::function<void(RowId)>& insertCallback_ = {})
            : query{query_.toUtf8()}
            , blobs{blobs_}
            , insertCallback{insertCallback_}
        {
        }
        Query(QString query_, const std::function<void(RowId)>& insertCallback_)
            : query{query_.toUtf8()}
            , insertCallback{insertCallback_}
        {
        }
        Query(QString query_, const std::function<void(const QVector<QVariant>&)>& rowCallback_)
            : query{query_.toUtf8()}
            , rowCallback{rowCallback_}
        {
        }
        Query(QString query_, QVector<QByteArray> blobs_,
              const std::function<void(const QVector<QVariant>&)>& rowCallback_)
            : query{query_.toUtf8()}
            , blobs{blobs_}
            , rowCallback{rowCallback_}
        {
        }
        Query() = default;

    private:
        QByteArray query;
        QVector<QByteArray> blobs;
        std::function<void(RowId)> insertCallback;
        std::function<void(const QVector<QVariant>&)> rowCallback;
        QVector<sqlite3_stmt*> statements;

        friend class RawDatabase;
    };

private:
    static void regexp(sqlite3_context* ctx, int argc, sqlite3_value** argv,
                       const QRegularExpression::PatternOptions cs);

    struct Transaction
    {
        QVector<Query> queries;
        std::atomic_bool* success = nullptr;
        std::atomic_bool* done = nullptr;
    };

public:
    enum class SqlCipherParams
    {
        // keep these sorted in upgrade order
        p3_0, // SQLCipher 3.0 default encryption params
              // SQLCipher 4.0 default params where SQLCipher 3.0 supports them, but 3.0 params
              // where not possible. We accidentally got to this state when attempting to update all
              // databases to 4.0 defaults even when using SQLCipher 3.x, but might as well keep
              // using these for people with SQLCipher 3.x.
        halfUpgradedTo4,
        p4_0 // SQLCipher 4.0 default encryption params
    };

    RawDatabase(const QString& path_, const QString& password, const QByteArray& salt);
    ~RawDatabase();
    bool isOpen();

    bool execNow(const QString& statement);
    bool execNow(const Query& statement);
    bool execNow(const QVector<Query>& statements);

    void execLater(const QString& statement);
    void execLater(const Query& statement);
    void execLater(const QVector<Query>& statements);

    void sync();

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
        assert(false);
        return {};
    }

public slots:
    bool setPassword(const QString& password);
    bool rename(const QString& newPath);
    bool remove();

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
    QQueue<Transaction> pendingTransactions;
    QMutex transactionsMutex;
    QString path;
    QByteArray currentSalt;
    QString currentHexKey;
};
