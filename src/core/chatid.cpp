/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/core/chatid.h"
#include <QByteArray>
#include <QHash>
#include <QString>
#include <cstdint>

/**
 * @brief The default constructor. Creates an empty id.
 */
ChatId::ChatId()
    : id()
{
}
ChatId::~ChatId() = default;

/**
 * @brief Constructs a ChatId from bytes.
 * @param rawId The bytes to construct the ChatId from.
 */
ChatId::ChatId(const QByteArray& rawId)
{
    id = QByteArray(rawId);
}

/**
 * @brief Compares the equality of the ChatId.
 * @param other ChatId to compare.
 * @return True if both ChatId are equal, false otherwise.
 */
bool ChatId::operator==(const ChatId& other) const
{
    return id == other.id;
}

/**
 * @brief Compares the inequality of the ChatId.
 * @param other ChatId to compare.
 * @return True if both ChatIds are not equal, false otherwise.
 */
bool ChatId::operator!=(const ChatId& other) const
{
    return id != other.id;
}

/**
 * @brief Compares two ChatIds
 * @param other ChatId to compare.
 * @return True if this ChatIds is less than the other ChatId, false otherwise.
 */
bool ChatId::operator<(const ChatId& other) const
{
    return id < other.id;
}

/**
 * @brief Converts the ChatId to a uppercase hex string.
 * @return QString containing the hex representation of the id
 */
QString ChatId::toString() const
{
    return QString::fromUtf8(id.toHex()).toUpper();
}

/**
 * @brief Returns a pointer to the raw id data.
 * @return Pointer to the raw id data, which is exactly `ChatId::getPkSize()`
 *         bytes long. Returns a nullptr if the ChatId is empty.
 */
const uint8_t* ChatId::getData() const
{
    if (id.isEmpty()) {
        return nullptr;
    }

    return reinterpret_cast<const uint8_t*>(id.constData());
}

/**
 * @brief Get a copy of the id
 * @return Copied id bytes
 */
QByteArray ChatId::getByteArray() const
{
    return QByteArray(id); // TODO: Is a copy really necessary?
}

/**
 * @brief Checks if the ChatId contains a id.
 * @return True if there is a id, False otherwise.
 */
bool ChatId::isEmpty() const
{
    return id.isEmpty();
}
