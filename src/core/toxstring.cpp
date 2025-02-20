/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "toxstring.h"

#include <QByteArray>
#include <QDebug>
#include <QSet>
#include <QString>

#include <cassert>
#include <climits>
#include <utility>

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
ToxString::ToxString(QByteArray text)
    : string(std::move(text))
{
}

/**
 * @brief Creates a ToxString from the representation used by c-toxcore.
 * @param text Pointer to the beginning of the text.
 * @param length Number of bytes to read from the beginning.
 */
ToxString::ToxString(const uint8_t* text, size_t length)
{
    if (length > SIZE_MAX){
        qCritical() << "The string has invalid length!";
        return;
    }
    // Some clients does not send the correct string end. Starting from  Qt6 we need to
    // make sure that we trim the string to its actual size.
    size_t actualLength = 0;
    for (actualLength=0; actualLength<length; actualLength++){
        if(*(text + actualLength) == 0)
            break;
    }
    string = QByteArray(reinterpret_cast<const char*>(text), actualLength);
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
 * @brief Interpret the string as UTF-8 encoded QString.
 *
 * Removes any non-printable characters from the string. This is a defense-in-depth measure to
 * prevent some potential security issues caused by bugs in client code or one of its dependencies.
 */
QString ToxString::getQString() const
{
    const auto tainted = QString::fromUtf8(string).toStdU32String();
    QSet<std::pair<QChar::Category, char32_t>> removed;
    std::u32string cleaned;
    std::copy_if(tainted.cbegin(), tainted.cend(), std::back_inserter(cleaned), [&removed](char32_t c) {
        const auto category = QChar::category(c);
        // Cf (Other_Format) is to allow skin-color modifiers for emojis.
        // We also allow newlines and tabs, which are Other_Control, and covered by isSpace.
        if (QChar::isPrint(c) || QChar::isSpace(c) || category == QChar::Category::Other_Format) {
            return true;
        }
        removed.insert({category, c});
        return false;
    });
    if (!removed.isEmpty()) {
        qWarning() << "Removed non-printable characters from a string:" << removed;
    }
    return QString::fromStdU32String(cleaned);
}

/**
 * @brief Returns the bytes verbatim as they were received from or will be sent to the network.
 *
 * No cleanup or interpretation is done here. The bytes are returned as they were received from the
 * network. Callers should be careful when processing these bytes. If UTF-8 messages are expected,
 * use getQString() instead.
 */
const QByteArray& ToxString::getBytes() const
{
    return string;
}
