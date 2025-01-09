/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */


#pragma once

#include "toxpk.h"

#include <QByteArray>
#include <QString>

#include <cstdint>

class ToxId
{
public:
    static constexpr int nospamSize = 4;
    static constexpr int nospamNumHexChars = nospamSize * 2;
    static constexpr int checksumSize = 2;
    static constexpr int checksumNumHexChars = checksumSize * 2;
    static constexpr int size = 38;
    static constexpr int numHexChars = size * 2;

    ToxId();
    ToxId(const ToxId& other);
    ToxId(ToxId&& other);
    explicit ToxId(const QString& id);
    explicit ToxId(const QByteArray& rawId);
    explicit ToxId(const uint8_t* rawId, int len);
    ToxId& operator=(const ToxId& other) = default;
    ToxId& operator=(ToxId&& other) = default;

    bool operator==(const ToxId& other) const;
    bool operator!=(const ToxId& other) const;
    QString toString() const;
    void clear();
    bool isValid() const;

    static bool isValidToxId(const QString& id);
    static bool isToxId(const QString& id);
    const uint8_t* getBytes() const;
    ToxPk getPublicKey() const;
    QString getNoSpamString() const;

private:
    void constructToxId(const QByteArray& rawId);

public:
    static const QRegularExpression ToxIdRegEx;

private:
    QByteArray toxId;
};
