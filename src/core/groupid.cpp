/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "groupid.h"

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class GroupId
 * @brief This class represents a long term persistent group identifier.
 */

/**
 * @brief The default constructor. Creates an empty Tox group ID.
 */
GroupId::GroupId()
    : ChatId()
{
}

/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from. The lenght must be exactly
 *              GroupId::size, else the GroupId will be empty.
 */
GroupId::GroupId(const QByteArray& rawId)
    : ChatId([rawId]() {
        assert(rawId.length() == size);
        return rawId;
    }())
{
}

/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from, will read exactly
 * GroupId::size from the specified buffer.
 */
GroupId::GroupId(const uint8_t* rawId)
    : ChatId(QByteArray(reinterpret_cast<const char*>(rawId), size))
{
}

/**
 * @brief Get size of public id in bytes.
 * @return Size of public id in bytes.
 */
int GroupId::getSize() const
{
    return size;
}

std::unique_ptr<ChatId> GroupId::clone() const
{
    return std::unique_ptr<ChatId>(new GroupId(*this));
}
