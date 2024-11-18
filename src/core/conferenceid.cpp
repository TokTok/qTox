/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "conferenceid.h"

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class ConferenceId
 * @brief This class represents a long term persistent conference identifier.
 */

/**
 * @brief The default constructor. Creates an empty Tox conference ID.
 */
ConferenceId::ConferenceId()
    : ChatId()
{
}

/**
 * @brief Constructs a ConferenceId from bytes.
 * @param rawId The bytes to construct the ConferenceId from. The lenght must be exactly
 *              ConferenceId::size, else the ConferenceId will be empty.
 */
ConferenceId::ConferenceId(const QByteArray& rawId)
    : ChatId([rawId]() {
        assert(rawId.length() == size);
        return rawId;
    }())
{
}

/**
 * @brief Constructs a ConferenceId from bytes.
 * @param rawId The bytes to construct the ConferenceId from, will read exactly
 * ConferenceId::size from the specified buffer.
 */
ConferenceId::ConferenceId(const uint8_t* rawId)
    : ChatId(QByteArray(reinterpret_cast<const char*>(rawId), size))
{
}

/**
 * @brief Get size of public id in bytes.
 * @return Size of public id in bytes.
 */
int ConferenceId::getSize() const
{
    return size;
}

std::unique_ptr<ChatId> ConferenceId::clone() const
{
    return std::unique_ptr<ChatId>(new ConferenceId(*this));
}
