/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "untrustedimage.h"

#include <QBuffer>
#include <QImageReader>
#include <QRegularExpression>
#include <QStringView>
#include <QXmlStreamReader>

namespace {
QImage decodeAs(const QByteArray& data, const char* format)
{
    QBuffer buffer;
    buffer.setData(data);
    if (!buffer.open(QIODevice::ReadOnly)) {
        return {};
    }

    QImageReader reader{&buffer, format};
    reader.setAutoDetectImageFormat(false);
    QImage image;
    if (!reader.read(&image)) {
        return {};
    }
    return image;
}

/**
 * @brief What content-based auto-detection picks for @p data.
 */
QByteArray autoDetectedFormat(const QByteArray& data)
{
    QBuffer buffer;
    buffer.setData(data);
    if (!buffer.open(QIODevice::ReadOnly)) {
        return {};
    }
    return QImageReader::imageFormat(&buffer);
}

const char* rasterFormat(const QByteArray& data)
{
    if (data.startsWith("\x89PNG\r\n\x1a\n")) {
        return "png";
    }
    if (data.startsWith("\xff\xd8\xff")) {
        return "jpeg";
    }
    if (data.startsWith("GIF87a") || data.startsWith("GIF89a")) {
        return "gif";
    }
    if (data.size() >= 12 && data.startsWith("RIFF") && data.mid(8, 4) == "WEBP") {
        return "webp";
    }
    if (data.startsWith("qoif")) {
        return "qoi";
    }
    return nullptr;
}

/**
 * @brief Checks an embedded image (SVG handler decodes with format auto-detection).
 */
bool isSafeDataImage(QStringView url)
{
    static const QRegularExpression dataImage{
        QStringLiteral(R"(^data:image/(png|jpeg|gif|webp);base64,([A-Za-z0-9+/=\s]*)$)")};
    const QRegularExpressionMatch match = dataImage.match(url.toString());
    if (!match.hasMatch()) {
        return false;
    }

    const QByteArray bytes = QByteArray::fromBase64(match.capturedView(2).toLatin1());
    const char* format = rasterFormat(bytes);
    return format != nullptr && autoDetectedFormat(bytes) == format
           && !decodeAs(bytes, format).isNull();
}

bool isSafeReference(QStringView ref)
{
    ref = ref.trimmed();
    return ref.startsWith(u'#') || isSafeDataImage(ref);
}

bool hasOnlyLocalUrls(QStringView text)
{
    qsizetype pos = 0;
    while ((pos = text.indexOf(u"url(", pos, Qt::CaseInsensitive)) >= 0) {
        pos += 4;
        const qsizetype end = text.indexOf(u')', pos);
        if (end < 0) {
            return false;
        }
        QStringView target = text.sliced(pos, end - pos).trimmed();
        if (target.startsWith(u'"') || target.startsWith(u'\'')) {
            target = target.sliced(1);
        }
        if (!target.startsWith(u'#')) {
            return false;
        }
        pos = end + 1;
    }
    return true;
}

bool isSafeCss(QStringView css)
{
    // CSS escapes could disguise "url(" or "@import".
    if (css.contains(u'\\') || css.contains(u"@import", Qt::CaseInsensitive)) {
        return false;
    }
    return hasOnlyLocalUrls(css);
}

/**
 * @brief Checks that an SVG document references nothing outside of itself.
 */
bool isSafeSvg(const QByteArray& data)
{
    QXmlStreamReader reader{data};
    bool sawRoot = false;
    while (!reader.atEnd()) {
        switch (reader.readNext()) {
        case QXmlStreamReader::DTD:
        case QXmlStreamReader::EntityReference:
        case QXmlStreamReader::ProcessingInstruction:
            return false;
        case QXmlStreamReader::StartElement: {
            if (!sawRoot && reader.name() != u"svg") {
                return false;
            }
            sawRoot = true;

            for (const QXmlStreamAttribute& attribute : reader.attributes()) {
                const QStringView value = attribute.value();
                if (attribute.name() == u"href" && !isSafeReference(value)) {
                    return false;
                }
                if (attribute.name() == u"style" ? !isSafeCss(value) : !hasOnlyLocalUrls(value)) {
                    return false;
                }
            }

            if (reader.name() == u"style"
                && !isSafeCss(reader.readElementText(QXmlStreamReader::IncludeChildElements))) {
                return false;
            }
            break;
        }
        default:
            break;
        }
    }
    return sawRoot && !reader.hasError();
}
} // namespace

namespace UntrustedImage {

QImage decode(const QByteArray& data)
{
    if (const char* format = rasterFormat(data)) {
        return decodeAs(data, format);
    }
    if (isSafeSvg(data)) {
        return decodeAs(data, "svg");
    }
    return {};
}

} // namespace UntrustedImage
