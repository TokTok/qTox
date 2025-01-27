/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "profile.h"
#include "settings.h"

#include "db/rawdatabase.h"
#include "db/upgrades/dbupgrader.h"
#include "src/core/chatid.h"
#include "src/core/toxpk.h"

#include <QDebug>

#include <cassert>
#include <tox/tox.h>
#include <utility>

namespace {
MessageState getMessageState(bool isPending, bool isBroken)
{
    assert(!(isPending && isBroken));
    MessageState messageState;

    if (isPending) {
        messageState = MessageState::pending;
    } else if (isBroken) {
        messageState = MessageState::broken;
    } else {
        messageState = MessageState::complete;
    }
    return messageState;
}

void addAuthorIdSubQuery(QString& queryString, QVector<QByteArray>& boundParams, const ToxPk& authorPk)
{
    boundParams.append(authorPk.getByteArray());
    queryString += "(SELECT id FROM authors WHERE public_key = ?)";
}

void addChatIdSubQuery(QString& queryString, QVector<QByteArray>& boundParams, const ChatId& chatId)
{
    boundParams.append(chatId.getByteArray());
    queryString += "(SELECT id FROM chats WHERE uuid = ?)";
}

RawDatabase::Query generateEnsurePkInChats(const ChatId& id)
{
    return RawDatabase::Query{
        QStringLiteral( //
            "INSERT OR IGNORE INTO chats (uuid) "
            "VALUES (?)"),
        {id.getByteArray()},
    };
}

RawDatabase::Query generateEnsurePkInAuthors(const ToxPk& pk)
{
    return RawDatabase::Query{
        QStringLiteral( //
            "INSERT OR IGNORE INTO authors (public_key) "
            "VALUES (?)"),
        {pk.getByteArray()},
    };
}

RawDatabase::Query generateUpdateAlias(const ToxPk& pk, const QString& dispName)
{
    QVector<QByteArray> boundParams;
    QString queryString = QStringLiteral( //
        "INSERT OR IGNORE INTO aliases (owner, display_name) VALUES (");
    addAuthorIdSubQuery(queryString, boundParams, pk);
    queryString += ", ?);";
    boundParams += dispName.toUtf8();
    return RawDatabase::Query{queryString, boundParams};
}

RawDatabase::Query generateHistoryTableInsertion(char type, const QDateTime& time, const ChatId& chatId)
{
    QVector<QByteArray> boundParams;
    QString queryString = //
        QStringLiteral(   //
            "INSERT INTO history (message_type, timestamp, chat_id) "
            "VALUES ('%1', %2, ")
            .arg(type)
            .arg(time.toMSecsSinceEpoch());
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += ");";
    return RawDatabase::Query{queryString, boundParams};
}

/**
 * @brief Generate query to insert new message in database
 * @param ChatId Chat ID to save.
 * @param message Message to save.
 * @param sender Sender to save.
 * @param time Time of message sending.
 * @param isDelivered True if message was already delivered.
 * @param dispName Name, which should be displayed.
 * @param insertIdCallback Function, called after query execution.
 */
std::vector<RawDatabase::Query>
generateNewTextMessageQueries(const ChatId& chatId, const QString& message, const ToxPk& sender,
                              const QDateTime& time, bool isDelivered, QString dispName,
                              std::function<void(RowId)> insertIdCallback)
{
    std::vector<RawDatabase::Query> queries;

    queries.emplace_back(generateEnsurePkInChats(chatId));
    queries.emplace_back(generateEnsurePkInAuthors(sender));
    queries.emplace_back(generateUpdateAlias(sender, dispName));
    queries.emplace_back(generateHistoryTableInsertion('T', time, chatId));

    QVector<QByteArray> boundParams;
    QString queryString = QStringLiteral( //
        "INSERT INTO text_messages (id, message_type, sender_alias, message) "
        "VALUES ( "
        "    last_insert_rowid(), "
        "    'T', "
        "    (SELECT id FROM aliases WHERE owner=");
    addAuthorIdSubQuery(queryString, boundParams, sender);
    queryString += " and display_name=?";
    boundParams += dispName.toUtf8();
    queryString += "), ?";
    boundParams += message.toUtf8();
    queryString += ");";
    queries.emplace_back(queryString, boundParams, insertIdCallback);

    if (!isDelivered) {
        queries.emplace_back(QStringLiteral( //
            "INSERT INTO faux_offline_pending (id) VALUES ("
            "    last_insert_rowid()"
            ");"));
    }

    return queries;
}

std::vector<RawDatabase::Query> generateNewSystemMessageQueries(const ChatId& chatId,
                                                                const SystemMessage& systemMessage)
{
    std::vector<RawDatabase::Query> queries;

    queries.emplace_back(generateEnsurePkInChats(chatId));
    queries.emplace_back(generateHistoryTableInsertion('S', systemMessage.timestamp, chatId));

    QVector<QByteArray> blobs;
    std::transform(systemMessage.args.begin(), systemMessage.args.end(), std::back_inserter(blobs),
                   [](const QString& s) { return s.toUtf8(); });

    queries.emplace_back( //
        QStringLiteral(   //
            "INSERT INTO system_messages (id, message_type, "
            "system_message_type, arg1, arg2, arg3, arg4)"
            "VALUES (last_insert_rowid(), 'S', %1, ?, ?, ?, ?)")
            .arg(static_cast<int>(systemMessage.messageType)),
        blobs);

    return queries;
}
} // namespace

/**
 * @class History
 * @brief Interacts with the profile database to save the chat history.
 *
 * @var QHash<QString, int64_t> History::peers
 * @brief Maps friend public keys to unique IDs by index.
 * Caches mappings to speed up message saving.
 */

FileDbInsertionData::FileDbInsertionData()
{
    static const int id = qRegisterMetaType<FileDbInsertionData>();
    (void)id;
}

/**
 * @brief Prepares the database to work with the history.
 * @param db This database will be prepared for use with the history.
 */
History::History(std::shared_ptr<RawDatabase> db_, Settings& settings_,
                 IMessageBoxManager& messageBoxManager)
    : db(std::move(db_))
    , settings(settings_)
{
    if (!isValid()) {
        qWarning() << "Database not open, init failed";
        return;
    }

    // foreign key support is not enabled by default, so needs to be enabled on every connection
    // support was added in sqlite 3.6.19, which is qTox's minimum supported version
    db->execNow("PRAGMA foreign_keys = ON;");

    const auto upgradeSucceeded = DbUpgrader::dbSchemaUpgrade(db, messageBoxManager);

    // dbSchemaUpgrade may have put us in an invalid state
    if (!upgradeSucceeded) {
        db.reset();
        return;
    }

    connect(this, &History::fileInserted, this, &History::onFileInserted);
}

History::~History()
{
    if (!isValid()) {
        return;
    }

    // We could have execLater requests pending with a lambda attached,
    // so clear the pending transactions first
    db->sync();
}

/**
 * @brief Checks if the database was opened successfully
 * @return True if database if opened, false otherwise.
 */
bool History::isValid()
{
    return db && db->isOpen();
}

/**
 * @brief Checks if a chat has history
 * @param chatId
 * @return True if it does, false otherwise.
 */
bool History::historyExists(const ChatId& chatId)
{
    if (historyAccessBlocked()) {
        return false;
    }

    return !getMessagesForChat(chatId, 0, 1).empty();
}

/**
 * @brief Erases all the chat history from the database.
 */
void History::eraseHistory()
{
    if (!isValid()) {
        return;
    }

    db->execNow( //
        "DELETE FROM faux_offline_pending;"
        "DELETE FROM broken_messages;"
        "DELETE FROM text_messages;"
        "DELETE FROM file_transfers;"
        "DELETE FROM system_messages;"
        "DELETE FROM history;"
        "DELETE FROM chats;"
        "DELETE FROM aliases;"
        "DELETE FROM authors;"
        "VACUUM;");
}

/**
 * @brief Erases the chat history of one chat.
 * @param chatId Chat ID to erase.
 */
void History::removeChatHistory(const ChatId& chatId)
{
    if (!isValid()) {
        return;
    }

    std::vector<RawDatabase::Query> queries;
    QVector<QByteArray> boundParams;
    QString queryString = QStringLiteral( //
        "DELETE FROM faux_offline_pending "
        "WHERE faux_offline_pending.id IN ( "
        "    SELECT faux_offline_pending.id FROM faux_offline_pending "
        "    LEFT JOIN history ON faux_offline_pending.id = history.id "
        "    WHERE chat_id=");
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += QStringLiteral(")");
    queries.emplace_back(queryString, boundParams);
    boundParams.clear();

    queryString = QStringLiteral( //
        "DELETE FROM broken_messages "
        "WHERE broken_messages.id IN ( "
        "    SELECT broken_messages.id FROM broken_messages "
        "    LEFT JOIN history ON broken_messages.id = history.id "
        "    WHERE chat_id=");
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += QStringLiteral(")");
    queries.emplace_back(queryString, boundParams);
    boundParams.clear();

    queryString = QStringLiteral( //
        "DELETE FROM text_messages "
        "WHERE id IN ("
        "   SELECT id from history "
        "   WHERE message_type = 'T' AND chat_id=");
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += QStringLiteral(")");
    queries.emplace_back(queryString, boundParams);
    boundParams.clear();

    queryString = QStringLiteral( //
        "DELETE FROM file_transfers "
        "WHERE id IN ( "
        "    SELECT id from history "
        "    WHERE message_type = 'F' AND chat_id=");
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += QStringLiteral(")");
    queries.emplace_back(queryString, boundParams);
    boundParams.clear();

    queryString = QStringLiteral( //
        "DELETE FROM system_messages "
        "WHERE id IN ( "
        "   SELECT id from history "
        "   WHERE message_type = 'S' AND chat_id=");
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += QStringLiteral(")");
    queries.emplace_back(queryString, boundParams);
    boundParams.clear();

    queryString = QStringLiteral("DELETE FROM history WHERE chat_id=");
    addChatIdSubQuery(queryString, boundParams, chatId);
    queries.emplace_back(queryString, boundParams);
    boundParams.clear();

    queryString = QStringLiteral("DELETE FROM chats WHERE id=");
    addChatIdSubQuery(queryString, boundParams, chatId);
    queries.emplace_back(queryString, boundParams);
    boundParams.clear();

    queries.emplace_back(QStringLiteral( //
        "DELETE FROM aliases WHERE id NOT IN ( "
        "   SELECT DISTINCT sender_alias FROM text_messages "
        "   UNION "
        "   SELECT DISTINCT sender_alias FROM file_transfers)"));

    queries.emplace_back(QStringLiteral( //
        "DELETE FROM authors WHERE id NOT IN ( "
        "   SELECT DISTINCT owner FROM aliases)"));

    if (!db->execNow(std::move(queries))) {
        qWarning() << "Failed to remove friend's history";
    } else {
        db->execNow(RawDatabase::Query{QStringLiteral("VACUUM")});
    }
}

void History::onFileInserted(RowId dbId, QByteArray fileId)
{
    auto& fileInfo = fileInfos[fileId];
    if (fileInfo.finished) {
        db->execLater(
            generateFileFinished(dbId, fileInfo.success, fileInfo.filePath, fileInfo.fileHash));
        fileInfos.remove(fileId);
    } else {
        fileInfo.finished = false;
        fileInfo.fileId = dbId;
    }
}

std::vector<RawDatabase::Query>
History::generateNewFileTransferQueries(const ChatId& chatId, const ToxPk& sender,
                                        const QDateTime& time, const QString& dispName,
                                        const FileDbInsertionData& insertionData)
{
    std::vector<RawDatabase::Query> queries;

    queries.emplace_back(generateEnsurePkInChats(chatId));
    queries.emplace_back(generateEnsurePkInAuthors(sender));
    queries.emplace_back(generateUpdateAlias(sender, dispName));
    queries.emplace_back(generateHistoryTableInsertion('F', time, chatId));

    const std::weak_ptr<History> weakThis = shared_from_this();
    auto fileId = insertionData.fileId;

    QString queryString;
    queryString += QStringLiteral( //
        "INSERT INTO file_transfers "
        "    (id, message_type, sender_alias, "
        "    file_restart_id, file_name, file_path, "
        "    file_hash, file_size, direction, file_state) "
        "VALUES ( "
        "    last_insert_rowid(), "
        "    'F', "
        "    (SELECT id FROM aliases WHERE owner=");
    QVector<QByteArray> boundParams;
    addAuthorIdSubQuery(queryString, boundParams, sender);
    queryString += " AND display_name=?";
    boundParams += dispName.toUtf8();
    queryString += "), ?";
    boundParams += insertionData.fileId;
    queryString += ", ?";
    boundParams += insertionData.fileName.toUtf8();
    queryString += ", ?";
    boundParams += insertionData.filePath.toUtf8();
    queryString += ", ?";
    boundParams += QByteArray();
    queryString += QStringLiteral(", %1, %2, %3);")
                       .arg(insertionData.size)
                       .arg(insertionData.direction)
                       .arg(ToxFile::CANCELED);
    queries.emplace_back(queryString, boundParams, [weakThis, fileId](RowId id) {
        auto pThis = weakThis.lock();
        if (pThis)
            emit pThis->fileInserted(id, fileId);
    });
    return queries;
}

RawDatabase::Query History::generateFileFinished(RowId id, bool success, const QString& filePath,
                                                 const QByteArray& fileHash)
{
    auto file_state = success ? ToxFile::FINISHED : ToxFile::CANCELED;
    if (filePath.length() != 0) {
        return RawDatabase::Query{
            QStringLiteral( //
                "UPDATE file_transfers "
                "SET file_state = %1, file_path = ?, file_hash = ?"
                "WHERE id = %2")
                .arg(file_state)
                .arg(id.get()),
            QVector<QByteArray>{filePath.toUtf8(), fileHash},
        };
    }
    return RawDatabase::Query{
        QStringLiteral( //
            "UPDATE file_transfers "
            "SET file_state = %1 "
            "WHERE id = %2")
            .arg(file_state)
            .arg(id.get()),
    };
}

void History::addNewFileMessage(const ChatId& chatId, const QByteArray& fileId,
                                const QString& fileName, const QString& filePath, int64_t size,
                                const ToxPk& sender, const QDateTime& time, const QString& dispName)
{
    if (historyAccessBlocked()) {
        return;
    }

    // This is an incredibly far from an optimal way of implementing this,
    // but given the frequency that people are going to be initiating a file
    // transfer we can probably live with it.

    // Since both inserting an alias for a user and inserting a file transfer
    // will generate new ids, there is no good way to inject both new ids into the
    // history query without refactoring our RawDatabase::Query and processor loops.

    // What we will do instead is chain callbacks to try to get reasonable behavior.
    // We can call the generateNewMessageQueries() fn to insert a message with an empty
    // message in it, and get the id with the callback. Once we have the id we can amend
    // the data to have our newly inserted file_id as well

    ToxFile::FileDirection direction;
    if (sender == chatId) {
        direction = ToxFile::RECEIVING;
    } else {
        direction = ToxFile::SENDING;
    }

    const std::weak_ptr<History> weakThis = shared_from_this();
    FileDbInsertionData insertionData;
    insertionData.fileId = fileId;
    insertionData.fileName = fileName;
    insertionData.filePath = filePath;
    insertionData.size = size;
    insertionData.direction = direction;

    db->execLater(generateNewFileTransferQueries(chatId, sender, time, dispName, insertionData));
}

void History::addNewSystemMessage(const ChatId& chatId, const SystemMessage& systemMessage)
{
    if (historyAccessBlocked())
        return;

    db->execLater(generateNewSystemMessageQueries(chatId, systemMessage));
}

/**
 * @brief Saves a chat message in the database.
 * @param chatId Chat ID to save.
 * @param message Message to save.
 * @param sender Sender to save.
 * @param time Time of message sending.
 * @param isDelivered True if message was already delivered.
 * @param dispName Name, which should be displayed.
 * @param insertIdCallback Function, called after query execution.
 */
void History::addNewMessage(const ChatId& chatId, const QString& message, const ToxPk& sender,
                            const QDateTime& time, bool isDelivered, QString dispName,
                            const std::function<void(RowId)>& insertIdCallback)
{
    if (historyAccessBlocked()) {
        return;
    }

    db->execLater(generateNewTextMessageQueries(chatId, message, sender, time, isDelivered,
                                                dispName, insertIdCallback));
}

void History::setFileFinished(const QByteArray& fileId, bool success, const QString& filePath,
                              const QByteArray& fileHash)
{
    if (historyAccessBlocked()) {
        return;
    }

    auto& fileInfo = fileInfos[fileId];
    if (fileInfo.fileId.get() == -1) {
        fileInfo.finished = true;
        fileInfo.success = success;
        fileInfo.filePath = filePath;
        fileInfo.fileHash = fileHash;
    } else {
        db->execLater(generateFileFinished(fileInfo.fileId, success, filePath, fileHash));
    }

    fileInfos.remove(fileId);
}

size_t History::getNumMessagesForChat(const ChatId& chatId)
{
    if (historyAccessBlocked()) {
        return 0;
    }

    return getNumMessagesForChatBeforeDate(chatId, QDateTime());
}

size_t History::getNumMessagesForChatBeforeDate(const ChatId& chatId, const QDateTime& date)
{
    if (historyAccessBlocked()) {
        return 0;
    }

    QString queryText = QStringLiteral( //
        "SELECT COUNT(history.id) "
        "FROM history "
        "JOIN chats ON chat_id = chats.id "
        "WHERE chats.uuid = ?");

    if (date.isNull()) {
        queryText += ";";
    } else {
        queryText += QString(" AND timestamp < %1;").arg(date.toMSecsSinceEpoch());
    }

    size_t numMessages = 0;
    auto rowCallback = [&numMessages](const QVector<QVariant>& row) {
        numMessages = row[0].toLongLong();
    };

    db->execNow({queryText, {chatId.getByteArray()}, rowCallback});

    return numMessages;
}

QList<History::HistMessage> History::getMessagesForChat(const ChatId& chatId, size_t firstIdx,
                                                        size_t lastIdx)
{
    if (historyAccessBlocked()) {
        return {};
    }

    QList<HistMessage> messages;

    auto rowCallback = [&chatId, &messages](const QVector<QVariant>& row) {
        // If the select statement is changed please update these constants
        constexpr auto messageOffset = 5;
        constexpr auto fileOffset = 6;
        constexpr auto senderOffset = 12;
        constexpr auto systemOffset = 14;

        auto it = row.begin();

        const auto id = RowId{(*it++).toLongLong()};
        const auto messageType = (*it++).toString();
        const auto timestamp = QDateTime::fromMSecsSinceEpoch((*it++).toLongLong());
        const auto isPending = !(*it++).isNull();
        const auto isBroken = !(*it++).isNull();
        const auto messageState = getMessageState(isPending, isBroken);

        // Intentionally arrange query so message types are at the end so we don't have to think
        // about the iterator jumping around after handling the different types.
        assert(messageType.size() == 1);
        switch (messageType[0].toLatin1()) {
        case 'T': {
            it = std::next(row.begin(), messageOffset);
            assert(!it->isNull());
            const auto messageContent = (*it++).toString();
            it = std::next(row.begin(), senderOffset);
            const auto senderKey = ToxPk{(*it++).toByteArray()};
            const auto senderName = QString::fromUtf8((*it++).toByteArray().replace('\0', ""));
            messages += HistMessage(id, messageState, timestamp, chatId.clone(), senderName,
                                    senderKey, messageContent);
            break;
        }
        case 'F': {
            it = std::next(row.begin(), fileOffset);
            assert(!it->isNull());
            const auto fileKind = TOX_FILE_KIND_DATA;
            const auto resumeFileId = (*it++).toByteArray();
            const auto fileName = (*it++).toString();
            const auto filePath = (*it++).toString();
            const auto filesize = (*it++).toLongLong();
            const auto direction = static_cast<ToxFile::FileDirection>((*it++).toLongLong());
            const auto status = static_cast<ToxFile::FileStatus>((*it++).toLongLong());

            ToxFile file(0, 0, fileName, filePath, filesize, direction);
            file.fileKind = fileKind;
            file.resumeFileId = resumeFileId;
            file.status = status;

            it = std::next(row.begin(), senderOffset);
            const auto senderKey = ToxPk{(*it++).toByteArray()};
            const auto senderName = QString::fromUtf8((*it++).toByteArray().replace('\0', ""));
            messages +=
                HistMessage(id, messageState, timestamp, chatId.clone(), senderName, senderKey, file);
            break;
        }
        default:
        case 'S':
            it = std::next(row.begin(), systemOffset);
            assert(!it->isNull());
            SystemMessage systemMessage;
            systemMessage.messageType = static_cast<SystemMessageType>((*it++).toLongLong());
            systemMessage.timestamp = timestamp;

            auto argEnd = std::next(it, systemMessage.args.size());
            std::transform(it, argEnd, systemMessage.args.begin(), [](const QVariant& arg) {
                return QString::fromUtf8(arg.toByteArray().replace('\0', ""));
            });
            it = argEnd;

            messages += HistMessage(id, timestamp, chatId.clone(), systemMessage);
            break;
        }
    };

    // Don't forget to update the rowCallback if you change the selected columns!
    QString queryString = QStringLiteral( //
        "SELECT\n"
        "    history.id,\n"
        "    history.message_type,\n"
        "    history.timestamp,\n"
        "    faux_offline_pending.id,\n"
        "    broken_messages.id,\n"
        "    text_messages.message,\n"
        "    file_transfers.file_restart_id,\n"
        "    file_transfers.file_name,\n"
        "    file_transfers.file_path,\n"
        "    file_transfers.file_size,\n"
        "    file_transfers.direction,\n"
        "    file_transfers.file_state,\n"
        "    authors.public_key as sender_key,\n"
        "    aliases.display_name,\n"
        "    system_messages.system_message_type,\n"
        "    system_messages.arg1,\n"
        "    system_messages.arg2,\n"
        "    system_messages.arg3,\n"
        "    system_messages.arg4\n"
        "FROM history "
        "LEFT JOIN text_messages ON history.id = text_messages.id "
        "LEFT JOIN file_transfers ON history.id = file_transfers.id "
        "LEFT JOIN system_messages ON system_messages.id == history.id "
        "LEFT JOIN aliases ON text_messages.sender_alias = aliases.id OR "
        "file_transfers.sender_alias = aliases.id "
        "LEFT JOIN authors ON aliases.owner = authors.id "
        "LEFT JOIN faux_offline_pending ON faux_offline_pending.id = history.id "
        "LEFT JOIN broken_messages ON broken_messages.id = history.id "
        "WHERE history.chat_id = ");
    QVector<QByteArray> boundParams;
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += QStringLiteral(" LIMIT %1 OFFSET %2;").arg(lastIdx - firstIdx).arg(firstIdx);
    db->execNow({queryString, boundParams, rowCallback});

    return messages;
}

QList<History::HistMessage> History::getUndeliveredMessagesForChat(const ChatId& chatId)
{
    if (historyAccessBlocked()) {
        return {};
    }

    QList<History::HistMessage> ret;
    auto rowCallback = [&chatId, &ret](const QVector<QVariant>& row) {
        auto it = row.begin();
        // dispName and message could have null bytes, QString::fromUtf8
        // truncates on null bytes so we strip them
        auto id = RowId{(*it++).toLongLong()};
        auto timestamp = QDateTime::fromMSecsSinceEpoch((*it++).toLongLong());
        auto isPending = !(*it++).isNull();
        auto isBroken = !(*it++).isNull();
        auto messageContent = (*it++).toString();
        auto senderKey = ToxPk{(*it++).toByteArray()};
        auto displayName = QString::fromUtf8((*it++).toByteArray().replace('\0', ""));

        const MessageState messageState = getMessageState(isPending, isBroken);

        ret +=
            {id, messageState, timestamp, chatId.clone(), displayName, senderKey, messageContent};
    };

    QString queryString = QStringLiteral( //
        "SELECT\n"
        "    history.id,\n"
        "    history.timestamp,\n"
        "    faux_offline_pending.id,\n"
        "    broken_messages.id,\n"
        "    text_messages.message,\n"
        "    authors.public_key as sender_key,\n"
        "    aliases.display_name\n"
        "FROM history "
        "JOIN text_messages ON history.id = text_messages.id "
        "JOIN aliases ON text_messages.sender_alias = aliases.id "
        "JOIN authors ON aliases.owner = authors.id "
        "JOIN faux_offline_pending ON faux_offline_pending.id = history.id "
        "LEFT JOIN broken_messages ON broken_messages.id = history.id "
        "WHERE history.chat_id = ");
    QVector<QByteArray> boundParams;
    addChatIdSubQuery(queryString, boundParams, chatId);
    queryString += QStringLiteral(" AND history.message_type = 'T';");
    db->execNow({queryString, boundParams, rowCallback});

    return ret;
}

/**
 * @brief Search phrase in chat messages
 * @param chatId Chat ID
 * @param from a date message where need to start a search
 * @param phrase what need to find
 * @param parameter for search
 * @return date of the message where the phrase was found
 */
QDateTime History::getDateWhereFindPhrase(const ChatId& chatId, const QDateTime& from,
                                          QString phrase, const ParameterSearch& parameter)
{
    if (historyAccessBlocked()) {
        return {};
    }

    QDateTime result;
    auto rowCallback = [&result](const QVector<QVariant>& row) {
        result = QDateTime::fromMSecsSinceEpoch(row[0].toLongLong());
    };

    phrase.replace("'", "''");

    QString message;

    switch (parameter.filter) {
    case FilterSearch::Register:
        message = QStringLiteral("text_messages.message LIKE '%%1%'").arg(phrase);
        break;
    case FilterSearch::WordsOnly:
        message = QStringLiteral("text_messages.message REGEXP '%1'")
                      .arg(SearchExtraFunctions::generateFilterWordsOnly(phrase).toLower());
        break;
    case FilterSearch::RegisterAndWordsOnly:
        message = QStringLiteral("REGEXPSENSITIVE(text_messages.message, '%1')")
                      .arg(SearchExtraFunctions::generateFilterWordsOnly(phrase));
        break;
    case FilterSearch::Regular:
        message = QStringLiteral("text_messages.message REGEXP '%1'").arg(phrase);
        break;
    case FilterSearch::RegisterAndRegular:
        message = QStringLiteral("REGEXPSENSITIVE(text_messages.message '%1')").arg(phrase);
        break;
    default:
        message = QStringLiteral("LOWER(text_messages.message) LIKE '%%1%'").arg(phrase.toLower());
        break;
    }

    QDateTime date = from;

    if (!date.isValid()) {
        date = QDateTime::currentDateTime();
    }

    if (parameter.period == PeriodSearch::AfterDate || parameter.period == PeriodSearch::BeforeDate) {

        date = parameter.date.startOfDay();
    }

    QString period;
    switch (parameter.period) {
    case PeriodSearch::WithTheFirst:
        period = QStringLiteral("ORDER BY timestamp ASC LIMIT 1;");
        break;
    case PeriodSearch::AfterDate:
        period = QStringLiteral("AND timestamp > '%1' ORDER BY timestamp ASC LIMIT 1;")
                     .arg(date.toMSecsSinceEpoch());
        break;
    case PeriodSearch::BeforeDate:
        period = QStringLiteral("AND timestamp < '%1' ORDER BY timestamp DESC LIMIT 1;")
                     .arg(date.toMSecsSinceEpoch());
        break;
    default:
        period = QStringLiteral("AND timestamp < '%1' ORDER BY timestamp DESC LIMIT 1;")
                     .arg(date.toMSecsSinceEpoch());
        break;
    }

    db->execNow(RawDatabase::Query( //
        QStringLiteral(             //
            "SELECT timestamp "
            "FROM history "
            "JOIN chats ON chat_id = chats.id "
            "JOIN text_messages ON history.id = text_messages.id "
            "WHERE chats.uuid = ? "
            "AND %1 "
            "%2")
            .arg(message)
            .arg(period),
        {chatId.getByteArray()}, rowCallback));

    return result;
}

/**
 * @brief Gets date boundaries in conversation with friendPk. History doesn't model conversation indexes,
 * but we can count messages between us and friendPk to effectively give us an index. This function
 * returns how many messages have happened between us <-> friendPk each time the date changes
 * @param[in] chatId ChatId of conversation to retrieve
 * @param[in] from Start date to look from
 * @param[in] maxNum Maximum number of date boundaries to retrieve
 * @note This API may seem a little strange, why not use QDate from and QDate to? The intent is to
 * have an API that can be used to get the first item after a date (for search) and to get a list
 * of date changes (for loadHistory). We could write two separate queries but the query is fairly
 * intricate compared to our other ones so reducing duplication of it is preferable.
 */
QList<History::DateIdx> History::getNumMessagesForChatBeforeDateBoundaries(const ChatId& chatId,
                                                                           const QDate& from,
                                                                           size_t maxNum)
{
    if (historyAccessBlocked()) {
        return {};
    }

    QList<DateIdx> dateIdxs;
    auto rowCallback = [&dateIdxs](const QVector<QVariant>& row) {
        DateIdx dateIdx;
        dateIdx.numMessagesIn = row[0].toLongLong();
        dateIdx.date =
            QDateTime::fromMSecsSinceEpoch(row[1].toLongLong() * 24 * 60 * 60 * 1000).date();
        dateIdxs.append(dateIdx);
    };

    // No guarantee that this is the most efficient way to do this...
    // We want to count messages that happened for a friend before a
    // certain date. We do this by re-joining our table a second time
    // but this time with the only filter being that our id is less than
    // the ID of the corresponding row in the table that is grouped by day
    auto countMessagesForFriend = QStringLiteral(
        "SELECT COUNT(*) - 1 "       // Count - 1 corresponds to 0 indexed message id for friend
        "FROM history countHistory " // Import unfiltered table as countHistory
        "JOIN chats ON chat_id = chats.id " // link chat_id to chat.id
        "WHERE chats.uuid = ?"              // filter this conversation
        "AND countHistory.id <= history.id" // and filter that our unfiltered table history id only has elements up to history.id
    );

    auto limitString = ((maxNum) != 0u) ? QString("LIMIT %1").arg(maxNum) : QString("");

    db->execNow(RawDatabase::Query( //
        QStringLiteral(             //
            "SELECT (%1), (timestamp / 1000 / 60 / 60 / 24) AS day "
            "FROM history "
            "JOIN chats ON chat_id = chats.id "
            "WHERE chats.uuid = ? "
            "AND timestamp >= %2 "
            "GROUP by day "
            "%3;")
            .arg(countMessagesForFriend)
            .arg(QDateTime(from.startOfDay()).toMSecsSinceEpoch())
            .arg(limitString),
        {chatId.getByteArray(), chatId.getByteArray()}, rowCallback));

    return dateIdxs;
}

/**
 * @brief Marks a message as delivered.
 * Removing message from the faux-offline pending messages list.
 *
 * @param id Message ID.
 */
void History::markAsDelivered(RowId messageId)
{
    if (historyAccessBlocked()) {
        return;
    }

    db->execLater(QStringLiteral("DELETE FROM faux_offline_pending WHERE id=%1;").arg(messageId.get()));
}

/**
 * @brief Determines if history access should be blocked
 * @return True if history should not be accessed
 */
bool History::historyAccessBlocked()
{
    if (!settings.getEnableLogging()) {
        qCritical() << "Blocked history access while history is disabled";
        return true;
    }

    if (!isValid()) {
        return true;
    }

    return false;
}

void History::markAsBroken(RowId messageId, BrokenMessageReason reason)
{
    if (!isValid()) {
        return;
    }

    std::vector<RawDatabase::Query> queries;
    queries.emplace_back( //
        QStringLiteral(   //
            "DELETE FROM faux_offline_pending WHERE id=%1;")
            .arg(messageId.get()));
    queries.emplace_back( //
        QStringLiteral(   //
            "INSERT INTO broken_messages (id, reason) "
            "VALUES (%1, %2);")
            .arg(messageId.get())
            .arg(static_cast<int64_t>(reason)));

    db->execLater(std::move(queries));
}
