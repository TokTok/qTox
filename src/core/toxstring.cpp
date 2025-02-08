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
#include <ctime>
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
    : ToxString(text, length, false)
{
}


/**
 * @brief Creates a ToxString from the representation used by c-toxcore.
 * @param text Pointer to the beginning of the text.
 * @param length Number of bytes to read from the beginning.
 * @param fix_trifa if true, fix TRIfA messages v3, containing suffix after \0.
 */
ToxString::ToxString(const uint8_t* text, size_t length, bool fix_trifa)
{
    if (length > SIZE_MAX) {
        qCritical() << "The string has invalid length!";
        return;
    }
    if (fix_trifa) {
        // Try to detect TRIfA message v3 format:
        // message itself
        // \0\0 TOX_MSGV3_GUARD = 2
        // random bytes TOX_MSGV3_MSGID_LENGTH = 32
        // time stamp TOX_MSGV3_TIMESTAMP_LENGTH = 4
        size_t actualLength = 0;
        for (actualLength = 0; actualLength < length; actualLength++) {
            if (*(text + actualLength) == 0)
                break;
        }
        // Only trim string if its suffix looks like a TRIfA signature.
        if (length - actualLength == 2 + 32 + 4 && *(text + actualLength + 1) == 0) {
            // Convert the uint8[4] to timestamp to string
            std::time_t trifa_timestamp = 0;
            for (int i = 4; i > 0; i--) {
                trifa_timestamp = trifa_timestamp << 8;
                trifa_timestamp += *(text + length - i);
            }
            string = QByteArray(reinterpret_cast<const char*>(text), actualLength);
            // Mark TRIFA suffix to remove it or replace by format symbol
            string.append("[TRIfA_SUFFIX]");
            // Skip two \0 symbols.
            // The string contains random characters and we need to sanitize it.
            QByteArray randomBt(reinterpret_cast<const char*>(text + actualLength + 2), 32);
            string.append(getQString(randomBt).toLocal8Bit().constData());
            // Convert time to string
            char strTime[std::size("yyyy-mm-ddThh:mm:ssZ")];
            std::strftime(std::data(strTime), std::size(strTime), "%Y-%m-%dT%H:%M:%SZ",
                          std::gmtime(&trifa_timestamp));
            string.append(strTime);
            string.append("[/TRIfA_SUFFIX]");
        } else {
            string = QByteArray(reinterpret_cast<const char*>(text), length);
        }
    } else {
        string = QByteArray(reinterpret_cast<const char*>(text), length);
    }
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
 * @return the string with non printable symbols removed.
 */
QString ToxString::getQString() const
{
    return getQString(string);
}


/**
 * @brief Interpret the string as UTF-8 encoded QString.
 *
 * Removes any non-printable characters from the string. This is a defense-in-depth measure to
 * prevent some potential security issues caused by bugs in client code or one of its dependencies.
 * @param stringVal the buffer with bytes.
 * @return the string with non printable symbols removed.
 */
QString ToxString::getQString(QByteArray stringVal) const
{
    const auto tainted = QString::fromUtf8(stringVal).toStdU32String();
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
