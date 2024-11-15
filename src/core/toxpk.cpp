/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "toxpk.h"
#include "chatid.h"

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class ToxPk
 * @brief This class represents a Tox Public Key, which is a part of Tox ID.
 */

/**
 * @brief The default constructor. Creates an empty Tox key.
 */
ToxPk::ToxPk()
    : ChatId()
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The lenght must be exactly
 *              ToxPk::size, else the ToxPk will be empty.
 */
ToxPk::ToxPk(const QByteArray& rawId)
    : ChatId([&rawId]() {
        assert(rawId.length() == size);
        return rawId;
    }())
{
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from, will read exactly
 * ToxPk::size from the specified buffer.
 */
ToxPk::ToxPk(const uint8_t* rawId)
    : ChatId(QByteArray(reinterpret_cast<const char*>(rawId), size))
{
}

/**
 * @brief Constructs a ToxPk from a QString.
 *
 * If the given pk isn't a valid Public Key a ToxPk with all zero bytes is created.
 *
 * @param pk Tox Pk string to convert to ToxPk object
 */
ToxPk::ToxPk(const QString& pk)
    : ChatId([&pk]() {
        if (pk.length() == numHexChars) {
            return QByteArray::fromHex(pk.toLatin1());
        } else {
            assert(!"ToxPk constructed with invalid length string");
            return QByteArray(); // invalid pk string
        }
    }())
{
}

/**
 * @brief Get size of public key in bytes.
 * @return Size of public key in bytes.
 */
int ToxPk::getSize() const
{
    return size;
}

std::unique_ptr<ChatId> ToxPk::clone() const
{
    return std::unique_ptr<ChatId>(new ToxPk(*this));
}
