/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
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
 * @param rawId The bytes to construct the ConferenceId from. The length must be exactly
 *              TOX_CONFERENCE_ID_SIZE, else the ConferenceId will be empty.
 */
ConferenceId::ConferenceId(const QByteArray& rawId)
    : ChatId([rawId]() {
        assert(rawId.length() == TOX_CONFERENCE_ID_SIZE);
        return rawId;
    }())
{
}

/**
 * @brief Constructs a ConferenceId from bytes.
 * @param rawId The bytes to construct the ConferenceId from, will read exactly
 * TOX_CONFERENCE_ID_SIZE from the specified buffer.
 */
ConferenceId::ConferenceId(const uint8_t* rawId)
    : ChatId(QByteArray(reinterpret_cast<const char*>(rawId), TOX_CONFERENCE_ID_SIZE))
{
}

/**
 * @brief Get size of public id in bytes.
 * @return Size of public id in bytes.
 */
int ConferenceId::getSize() const
{
    return TOX_CONFERENCE_ID_SIZE;
}

std::unique_ptr<ChatId> ConferenceId::clone() const
{
    return std::make_unique<ConferenceId>(*this);
}
