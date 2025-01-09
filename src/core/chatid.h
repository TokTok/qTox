/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>

#include <cstdint>
#include <memory>

class ChatId
{
public:
    virtual ~ChatId();
    ChatId(const ChatId&) = default;
    ChatId& operator=(const ChatId&) = default;
    ChatId(ChatId&&) = default;
    ChatId& operator=(ChatId&&) = default;
    bool operator==(const ChatId& other) const;
    bool operator!=(const ChatId& other) const;
    bool operator<(const ChatId& other) const;
    QString toString() const;
    QByteArray getByteArray() const;
    const uint8_t* getData() const;
    bool isEmpty() const;
    virtual int getSize() const = 0;
    virtual std::unique_ptr<ChatId> clone() const = 0;

protected:
    ChatId();
    explicit ChatId(const QByteArray& rawId);
    QByteArray id;
};

inline uint qHash(const ChatId& id)
{
    return qHash(id.getByteArray());
}

inline bool operator==(const std::reference_wrapper<const ChatId> lhs,
                       const std::reference_wrapper<const ChatId> rhs)
{
    return lhs.get() == rhs.get();
}

using ChatIdPtr = std::shared_ptr<const ChatId>;
