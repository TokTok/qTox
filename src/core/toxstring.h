/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QByteArray>
#include <QString>

#include <cstdint>

class ToxString
{
public:
    explicit ToxString(const QString& text);
    explicit ToxString(QByteArray text);
    ToxString(const uint8_t* text, size_t length);
    ToxString(const uint8_t* text, size_t length, bool fix_trifa);

    const uint8_t* data() const;
    size_t size() const;
    QString getQString() const;
    QString getQString(QByteArray stringVal) const;
    const QByteArray& getBytes() const;

private:
    QByteArray string;
};
