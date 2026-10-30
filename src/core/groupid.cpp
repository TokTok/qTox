/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "groupid.h"

#include <QByteArray>
#include <QString>

#include <cassert>

/**
 * @class GroupId
 * @brief This class represents a long term persistent group chat identifier
 *        (the "Chat ID" of an NGC group).
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
 * @param rawId The bytes to construct the GroupId from. The length must be exactly
 *              TOX_GROUP_CHAT_ID_SIZE, else the GroupId will be empty.
 */
GroupId::GroupId(const QByteArray& rawId)
    : ChatId([rawId]() {
        assert(rawId.length() == TOX_GROUP_CHAT_ID_SIZE);
        return rawId;
    }())
{
}

/**
 * @brief Constructs a GroupId from bytes.
 * @param rawId The bytes to construct the GroupId from, will read exactly
 * TOX_GROUP_CHAT_ID_SIZE from the specified buffer.
 */
GroupId::GroupId(const uint8_t* rawId)
    : ChatId(QByteArray(reinterpret_cast<const char*>(rawId), TOX_GROUP_CHAT_ID_SIZE))
{
}

/**
 * @brief Get size of public id in bytes.
 * @return Size of public id in bytes.
 */
int GroupId::getSize() const
{
    return TOX_GROUP_CHAT_ID_SIZE;
}

std::unique_ptr<ChatId> GroupId::clone() const
{
    return std::make_unique<GroupId>(*this);
}
