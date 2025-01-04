/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "util/strongtype.h"

#include <QMetaType>
#include <QVariant>
#include <QVector>

#include <functional>
#include <vector>

struct sqlite3_stmt;

using RowId = NamedType<int64_t, struct RowIdTag, Orderable>;
Q_DECLARE_METATYPE(RowId)

/**
 * @brief Implements a low level RAII interface to a SQLCipher (SQlite3) database.
 *
 * Thread-safe, does all database operations on a worker thread.
 * The queries must not contain transaction commands (BEGIN/COMMIT/...) or the behavior is
 * undefined.
 *
 * @var QMutex RawDatabaseImpl::transactionsMutex;
 * @brief Protects pendingTransactions
 */
class RawDatabase
{
public:
    virtual ~RawDatabase();

public:
    /**
     * @brief A query to be executed by the database.
     *
     * Can be composed of one or more SQL statements in the query,
     * optional BLOB parameters to be bound, and callbacks fired when the query is executed
     * Calling any database method from a query callback is undefined behavior.
     */
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
        Query(const Query&) = delete;
        Query& operator=(const Query&) = delete;
        Query(Query&&) = default;
        Query& operator=(Query&&) = default;

    private:
        /** @brief UTF-8 query string */
        QByteArray query;
        /** @brief Bound data blobs */
        QVector<QByteArray> blobs;
        /** @brief Called after execution with the last insert rowid */
        std::function<void(RowId)> insertCallback;
        /** @brief Called during execution for each row */
        std::function<void(const QVector<QVariant>&)> rowCallback;
        /** @brief Statements to be compiled from the query */
        QVector<sqlite3_stmt*> statements;

        friend class RawDatabaseImpl;
    };

    virtual bool isOpen() = 0;

    virtual bool execNow(const QString& statement) = 0;
    virtual bool execNow(Query statement) = 0;
    virtual bool execNow(std::vector<Query> statements) = 0;

    virtual void execLater(const QString& statement) = 0;
    virtual void execLater(Query statement) = 0;
    virtual void execLater(std::vector<Query> statements) = 0;

    virtual void sync() = 0;

    static std::shared_ptr<RawDatabase> open(const QString& path, const QString& password,
                                             const QByteArray& salt);

public:
    virtual bool setPassword(const QString& password) = 0;
    virtual bool rename(const QString& newPath) = 0;
    virtual bool remove() = 0;
};
