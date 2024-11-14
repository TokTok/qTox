/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "toxstring.h"

#include <QByteArray>
#include <QString>

#include <cassert>
#include <climits>

/**
 * @class ToxString
 * @brief Helper to convert safely between strings in the c-toxcore representation and QString.
 */

/**
 * @brief Creates a ToxString from a QString.
 * @param string Input text.
 */
ToxString::ToxString(const QString& text)
    : ToxString(text.toUtf8())
{
}

/**
 * @brief Creates a ToxString from bytes in a QByteArray.
 * @param text Input text.
 */
ToxString::ToxString(const QByteArray& text)
    : string(text)
{
}

/**
 * @brief Creates a ToxString from the representation used by c-toxcore.
 * @param text Pointer to the beginning of the text.
 * @param length Number of bytes to read from the beginning.
 */
ToxString::ToxString(const uint8_t* text, size_t length)
{
    assert(length <= INT_MAX);
    string = QByteArray(reinterpret_cast<const char*>(text), length);
}

/**
 * @brief Returns a pointer to the beginning of the string data.
 * @return Pointer to the beginning of the string data.
 */
const uint8_t* ToxString::data() const
{
    return reinterpret_cast<const uint8_t*>(string.constData());
}

/**
 * @brief Get the number of bytes in the string.
 * @return Number of bytes in the string.
 */
size_t ToxString::size() const
{
    return string.size();
}

/**
 * @brief Gets the string as QString.
 * @return QString representation of the string.
 */
QString ToxString::getQString() const
{
    return QString::fromUtf8(string);
}

/**
 * @brief getBytes Gets the bytes of the string.
 * @return Bytes of the string as QByteArray.
 */
QByteArray ToxString::getBytes() const
{
    return QByteArray(string);
}
