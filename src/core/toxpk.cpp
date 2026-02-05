/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "toxpk.h"

#include "chatid.h"

#include <QByteArray>
#include <QString>

/**
 * @class ToxPk
 * @brief This class represents a Tox Public Key, which is a part of Tox ID.
 */

/**
 * @brief The default constructor. Creates an empty Tox key.
 */
ToxPk::ToxPk() {}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from. The length must be exactly
 *              ToxPk::size, else the ToxPk will be empty.
 */
ToxPk::ToxPk(QByteArray rawId)
    : ChatId(std::move(rawId))
{
    if (id.length() != size) {
        qCritical("ToxPk constructed with invalid length (%u instead of %d)",
                  static_cast<uint>(id.length()), size);
    }
}

/**
 * @brief Constructs a ToxPk from bytes.
 * @param rawId The bytes to construct the ToxPk from, will read exactly
 * ToxPk::size from the specified buffer.
 */
ToxPk::ToxPk(const uint8_t* rawId)
    : ToxPk(QByteArray(reinterpret_cast<const char*>(rawId), size))
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
    : ToxPk([&pk]() {
        if (pk.length() != numHexChars) {
            qCritical("ToxPk constructed with invalid length string (%u instead of %d)",
                      static_cast<uint>(pk.length()), numHexChars);
        }
        return QByteArray::fromHex(pk.toLatin1());
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
    return std::make_unique<ToxPk>(*this);
}
