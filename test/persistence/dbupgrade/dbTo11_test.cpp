/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/core/toxpk.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/persistence/db/upgrades/dbto11.h"

#include "dbutility/dbutility.h"

#include <QByteArray>
#include <QTemporaryFile>
#include <QTest>

namespace {
const auto selfPk = ToxPk{QByteArray(32, 0)};
const auto aPk = ToxPk{QByteArray(32, '=')};
const auto bPk = ToxPk{QByteArray(32, 2)};
const auto cPk = ToxPk{QByteArray(32, 3)};
const auto selfName = QStringLiteral("Self");
const auto aName = QStringLiteral("Alice");
const auto bName = QStringLiteral("Bob");
const auto selfAliasId = 1;
const auto aPeerId = 2;
const auto aPeerDuplicateId = 5;
const auto aChatId = 3;
const auto aAliasId = 2;
const auto aAliasDuplicateId = 4;
const auto bPeerId = 3;
const auto bChatId = 1;
const auto bAliasId = 3;
const auto cPeerId = 4;
const auto cChatId = 2;

void appendAddPeersQueries(std::vector<RawDatabase::Query>& setupQueries)
{
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO peers (id, public_key) VALUES (1, ?)"),
                           {selfPk.toString().toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO peers (id, public_key) VALUES (2, ?)"),
                           {aPk.toString().toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO peers (id, public_key) VALUES (3, ?)"),
                           {bPk.toString().toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO peers (id, public_key) VALUES (4, ?)"),
                           {cPk.toString().toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO peers (id, public_key) VALUES (5, ?)"),
                           {aPk.toString().toLower().toUtf8()}});
}

void appendVerifyChatsQueries(std::vector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM chats"),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 3); }});

    struct Functor
    {
        const std::vector<QByteArray> chatIds{aPk.getByteArray(), bPk.getByteArray(),
                                              cPk.getByteArray()};
        void operator()(const QVector<QVariant>& row)
        {
            QVERIFY(std::find(chatIds.begin(), chatIds.end(), row[0].toByteArray()) != chatIds.end());
        }
    };

    verifyQueries.emplace_back(RawDatabase::Query{QStringLiteral("SELECT uuid FROM chats"), Functor()});
}

void appendVerifyAuthorsQueries(std::vector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM authors"),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 3); }});

    struct Functor
    {
        const std::vector<QByteArray> authorIds{selfPk.getByteArray(), aPk.getByteArray(),
                                                bPk.getByteArray()};
        void operator()(const QVector<QVariant>& row)
        {
            QVERIFY(std::find(authorIds.begin(), authorIds.end(), row[0].toByteArray())
                    != authorIds.end());
        }
    };

    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT public_key FROM authors"), Functor()});
}

void appendAddAliasesQueries(std::vector<RawDatabase::Query>& setupQueries)
{
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral(
                               "INSERT INTO aliases (id, owner, display_name) VALUES (1, 1, ?)"),
                           {selfName.toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral(
                               "INSERT INTO aliases (id, owner, display_name) VALUES (2, 2, ?)"),
                           {aName.toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral(
                               "INSERT INTO aliases (id, owner, display_name) VALUES (3, 3, ?)"),
                           {bName.toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral(
                               "INSERT INTO aliases (id, owner, display_name) VALUES (4, 5, ?)"),
                           {aName.toUtf8()}});
}

void appendVerifyAliasesQueries(std::vector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM aliases"),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 3); }});

    struct Functor
    {
        const std::vector<QByteArray> names{selfName.toUtf8(), aName.toUtf8(), bName.toUtf8()};
        void operator()(const QVector<QVariant>& row)
        {
            QVERIFY(std::find(names.begin(), names.end(), row[0].toByteArray()) != names.end());
        }
    };

    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT display_name FROM aliases"), Functor()});
}

void appendAddAChatMessagesQueries(std::vector<RawDatabase::Query>& setupQueries)
{
    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (1, 'T', 0, '%1')")
            .arg(aPeerId)});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO text_messages (id, message_type, "
                                          "sender_alias, message) VALUES (1, 'T', '%1', ?)")
                               .arg(aAliasId),
                           {QStringLiteral("Message 1 from A to Self").toUtf8()}});

    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (2, 'T', 0, '%1')")
            .arg(aPeerId)});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO text_messages (id, message_type, "
                                          "sender_alias, message) VALUES (2, 'T', '%1', ?)")
                               .arg(aAliasDuplicateId),
                           {QStringLiteral("Message 2 from A to Self").toUtf8()}});

    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (10, 'F', 0, '%1')")
            .arg(aPeerDuplicateId)});
    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO file_transfers (id, message_type, sender_alias, file_restart_id, "
            "file_name, file_path, file_hash, file_size, direction, file_state) "
            "VALUES(10, 'F', '%1', ?, 'dummy name', 'dummy/path', ?, 1024, 1, 5)")
            .arg(aAliasId),
        {QByteArray(32, 1), QByteArray(32, 2)}});
}

void appendVerifyAChatMessagesQueries(std::vector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM history WHERE chat_id = '%1'").arg(aChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 3); }});
    verifyQueries.emplace_back(RawDatabase::Query{
        QStringLiteral("SELECT COUNT(*) FROM text_messages WHERE sender_alias = '%1'").arg(aAliasId),
        [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 2); }});
    verifyQueries.emplace_back(RawDatabase::Query{
        QStringLiteral("SELECT COUNT(*) FROM file_transfers WHERE sender_alias = '%1'").arg(aAliasId),
        [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 1); }});
}

void appendAddBChatMessagesQueries(std::vector<RawDatabase::Query>& setupQueries)
{
    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (3, 'T', 0, '%1')")
            .arg(bPeerId)});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO text_messages (id, message_type, "
                                          "sender_alias, message) VALUES (3, 'T', '%1', ?)")
                               .arg(bAliasId),
                           {QStringLiteral("Message 1 from B to Self").toUtf8()}});

    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (4, 'T', 0, '%1')")
            .arg(bPeerId)});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO text_messages (id, message_type, "
                                          "sender_alias, message) VALUES (4, 'T', '%1', ?)")
                               .arg(selfAliasId),
                           {QStringLiteral("Message 1 from Self to B").toUtf8()}});

    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (5, 'T', 0, '%1')")
            .arg(bPeerId)});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO text_messages (id, message_type, "
                                          "sender_alias, message) VALUES (5, 'T', '%1', ?)")
                               .arg(selfAliasId),
                           {QStringLiteral("Pending message 1 from Self to B").toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO faux_offline_pending (id) VALUES (5)")});

    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (8, 'S', 0, '%1')")
            .arg(bPeerId)});
    setupQueries.emplace_back(RawDatabase::Query{QStringLiteral(
        "INSERT INTO system_messages (id, message_type, system_message_type) VALUES (8, 'S', 1)")});

    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (9, 'F', 0, '%1')")
            .arg(bPeerId)});
    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO file_transfers (id, message_type, sender_alias, file_restart_id, "
            "file_name, file_path, file_hash, file_size, direction, file_state) "
            "VALUES(9, 'F', '%1', ?, 'dummy name', 'dummy/path', ?, 1024, 0, 5)")
            .arg(selfAliasId),
        {QByteArray(32, 1), QByteArray(32, 2)}});
}

void appendVerifyBChatMessagesQueries(std::vector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM history WHERE chat_id = '%1'").arg(bChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 5); }});
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM history JOIN text_messages ON "
                                          "history.id = text_messages.id WHERE chat_id = '%1'")
                               .arg(bChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 3); }});
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral(
                               "SELECT COUNT(*) FROM history JOIN faux_offline_pending ON "
                               "history.id = faux_offline_pending.id WHERE chat_id = '%1'")
                               .arg(bChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 1); }});
    verifyQueries.emplace_back(RawDatabase::Query{
        QStringLiteral("SELECT COUNT(*) FROM file_transfers WHERE sender_alias = '%1'").arg(selfAliasId),
        [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 1); }});
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM history JOIN system_messages ON "
                                          "history.id = system_messages.id WHERE chat_id = '%1'")
                               .arg(bChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 1); }});
}

void appendAddCChatMessagesQueries(std::vector<RawDatabase::Query>& setupQueries)
{
    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (6, 'T', 0, '%1')")
            .arg(cPeerId)});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO text_messages (id, message_type, "
                                          "sender_alias, message) VALUES (6, 'T', '%1', ?)")
                               .arg(selfAliasId),
                           {QStringLiteral("Message 1 from Self to B").toUtf8()}});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO broken_messages (id) VALUES (6)")});

    setupQueries.emplace_back(RawDatabase::Query{
        QStringLiteral(
            "INSERT INTO history (id, message_type, timestamp, chat_id) VALUES (7, 'T', 0, '%1')")
            .arg(cPeerId)});
    setupQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("INSERT INTO text_messages (id, message_type, "
                                          "sender_alias, message) VALUES (7, 'T', '%1', ?)")
                               .arg(selfAliasId),
                           {QStringLiteral("Message 1 from Self to B").toUtf8()}});
}

void appendVerifyCChatMessagesQueries(std::vector<RawDatabase::Query>& verifyQueries)
{
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM history WHERE chat_id = '%1'").arg(cChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 2); }});
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM history JOIN broken_messages ON "
                                          "history.id = broken_messages.id WHERE chat_id = '%1'")
                               .arg(cChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 1); }});
    verifyQueries.emplace_back(
        RawDatabase::Query{QStringLiteral("SELECT COUNT(*) FROM history JOIN text_messages ON "
                                          "history.id = text_messages.id WHERE chat_id = '%1'")
                               .arg(cChatId),
                           [&](const QVector<QVariant>& row) { QVERIFY(row[0].toLongLong() == 2); }});
}

void appendAddHistoryQueries(std::vector<RawDatabase::Query>& setupQueries)
{
    appendAddAChatMessagesQueries(setupQueries);
    appendAddBChatMessagesQueries(setupQueries);
    appendAddCChatMessagesQueries(setupQueries);
}

void appendVerifyHistoryQueries(std::vector<RawDatabase::Query>& verifyQueries)
{
    appendVerifyAChatMessagesQueries(verifyQueries);
    appendVerifyBChatMessagesQueries(verifyQueries);
    appendVerifyCChatMessagesQueries(verifyQueries);
}
} // namespace

class Test10to11 : public QObject
{
    Q_OBJECT

private slots:
    void test10to11();

private:
    QTemporaryFile testDatabaseFile;
};

void Test10to11::test10to11()
{
    QVERIFY(testDatabaseFile.open());
    testDatabaseFile.close();
    auto db = RawDatabase::open(testDatabaseFile.fileName(), {}, {});
    QVERIFY(db->execNow(RawDatabase::Query{QStringLiteral("PRAGMA foreign_keys = ON;")}));
    createSchemaAtVersion(db, DbUtility::schema10);
    std::vector<RawDatabase::Query> setupQueries;
    appendAddPeersQueries(setupQueries);
    appendAddAliasesQueries(setupQueries);
    appendAddHistoryQueries(setupQueries);
    QVERIFY(db->execNow(std::move(setupQueries)));
    QVERIFY(DbTo11::dbSchema10to11(*db));
    verifyDb(db, DbUtility::schema11);
    std::vector<RawDatabase::Query> verifyQueries;
    appendVerifyChatsQueries(verifyQueries);
    appendVerifyAuthorsQueries(verifyQueries);
    appendVerifyAliasesQueries(verifyQueries);
    appendVerifyHistoryQueries(verifyQueries);
    QVERIFY(db->execNow(std::move(verifyQueries)));
}

QTEST_GUILESS_MAIN(Test10to11)
#include "dbTo11_test.moc"
