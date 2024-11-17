/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "smileypack.h"
#include "src/persistence/ismileysettings.h"

#include <QDir>
#include <QDomElement>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>

#if defined(Q_OS_FREEBSD)
#include <locale.h>
#endif

/**
 * @class SmileyPack
 * @brief Maps emoticons to smileys.
 *
 * @var SmileyPack::filenameTable
 * @brief Matches an emoticon to its corresponding smiley ie. ":)" -> "happy.png"
 *
 * @var SmileyPack::iconCache
 * @brief representation of a smiley ie. "happy.png" -> data
 *
 * @var SmileyPack::emoticons
 * @brief {{ ":)", ":-)" }, {":(", ...}, ... }
 *
 * @var SmileyPack::path
 * @brief directory containing the cfg and image files
 *
 * @var SmileyPack::defaultPaths
 * @brief Contains all directories where smileys could be found
 */

namespace {
QStringList loadDefaultPaths();

const QStringList DEFAULT_PATHS = loadDefaultPaths();

const QString RICH_TEXT_PATTERN = QStringLiteral("<img title=\"%1\" src=\"key:%1\"\\>");

const QString EMOTICONS_FILE_NAME = QStringLiteral("emoticons.xml");

constexpr int CLEANUP_TIMEOUT = 5 * 60 * 1000; // 5 minutes

/**
 * @brief Construct list of standard directories with "emoticons" sub dir, whether these directories
 * exist or not
 * @return Constructed list of default emoticons directories
 */
QStringList loadDefaultPaths()
{
#if defined(Q_OS_FREEBSD)
    // TODO: Remove when will be fixed.
    // Workaround to fix https://bugreports.qt.io/browse/QTBUG-57522
    setlocale(LC_ALL, "");
#endif
    const QString EMOTICONS_SUB_PATH = QDir::separator() + QStringLiteral("emoticons");
    QStringList paths{":/smileys", "~/.kde4/share/emoticons", "~/.kde/share/emoticons",
                      EMOTICONS_SUB_PATH};

    // qTox exclusive emoticons
    QStandardPaths::StandardLocation location;
    location = QStandardPaths::AppDataLocation;

    QStringList locations = QStandardPaths::standardLocations(location);
    // system wide emoticons
    locations.append(QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation));
    for (QString qtoxPath : locations) {
        qtoxPath.append(EMOTICONS_SUB_PATH);
        if (!paths.contains(qtoxPath)) {
            paths.append(qtoxPath);
        }
    }

    return paths;
}

bool isAscii(const QString& string)
{
    constexpr auto asciiExtMask = 0x80;

    return (string.toUtf8()[0] & asciiExtMask) == 0;
}

/**
 * Single-character ASCII sequences like "0" can be emojis, but we want to avoid matching them
 * directly. For these, we require ":" before and after the sequence, e.g. ":0:".
 */
QString singleCharAsciiEmoticon(const QString& emoji)
{
    if (emoji.length() == 1 && isAscii(emoji)) {
        return QStringLiteral(":") % emoji % QStringLiteral(":");
    }

    return emoji;
}

} // namespace

SmileyPack::SmileyPack(ISmileySettings& settings_)
    : cleanupTimer{new QTimer(this)}
    , settings{settings_}
{
    load(settings.getSmileyPack());

    settings.connectTo_smileyPackChanged(this, [this](const QString& filename) { load(filename); });
    connect(cleanupTimer, &QTimer::timeout, this, &SmileyPack::cleanupIconsCache);
    cleanupTimer->start(CLEANUP_TIMEOUT);
}

SmileyPack::~SmileyPack() = default;

/**
 * @brief Wraps passed string into smiley HTML image reference
 * @param key Describes which smiley is needed
 * @return Key that wrapped into image ref
 */
QString SmileyPack::getAsRichText(const QString& key)
{
    return RICH_TEXT_PATTERN.arg(key);
}

void SmileyPack::cleanupIconsCache()
{
    const QMutexLocker<QMutex> locker(&loadingMutex);
    for (auto it = cachedIcon.begin(); it != cachedIcon.end();) {
        const std::shared_ptr<QIcon>& icon = it->second;
        if (icon.use_count() == 1) {
            it = cachedIcon.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * @brief Does the same as listSmileyPaths, but with default paths
 */
QList<QPair<QString, QString>> SmileyPack::listSmileyPacks()
{
    return listSmileyPacks(DEFAULT_PATHS);
}

/**
 * @brief Searches all files called "emoticons.xml" within the every passed path in the depth of 2
 * @param paths Paths where to search for file
 * @return Vector of pairs: {directoryName, absolutePathToFile}
 */
QList<QPair<QString, QString>> SmileyPack::listSmileyPacks(const QStringList& paths)
{
    QList<QPair<QString, QString>> smileyPacks;
    const QString homePath = QDir::homePath();
    for (QString path : paths) {
        if (path.startsWith('~')) {
            path.replace(0, 1, homePath);
        }

        QDir dir(path);
        if (!dir.exists()) {
            continue;
        }

        for (const QString& subdirectory : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            dir.cd(subdirectory);
            if (dir.exists(EMOTICONS_FILE_NAME)) {
                const QString absPath = dir.absolutePath() + QDir::separator() + EMOTICONS_FILE_NAME;
                const QPair<QString, QString> p{dir.dirName(), absPath};
                if (!smileyPacks.contains(p)) {
                    smileyPacks.append(p);
                }
            }

            dir.cdUp();
        }
    }

    return smileyPacks;
}

/**
 * @brief Load smiley pack.
 *
 * @param filename Filename of smiley pack.
 */
void SmileyPack::load(const QString& filename)
{
    const QMutexLocker<QMutex> locker(&loadingMutex);

    QFile xmlFile(filename);
    if (!xmlFile.exists() || !xmlFile.open(QIODevice::ReadOnly)) {
        qWarning() << "SmileyPack: Could not open file" << filename;
        return;
    }

    QDomDocument doc;
    doc.setContent(xmlFile.readAll());
    xmlFile.close();

    /* parse the cfg file
     * sample:
     * <?xml version='1.0'?>
     * <messaging-emoticon-map>
     *   <emoticon file="smile.png" >
     *       <string>:)</string>
     *       <string>:-)</string>
     *   </emoticon>
     *   <emoticon file="sad.png" >
     *       <string>:(</string>
     *       <string>:-(</string>
     *   </emoticon>
     * </messaging-emoticon-map>
     */

    path = QFileInfo(filename).absolutePath();
    const QDomNodeList emoticonElements = doc.elementsByTagName("emoticon");
    const QString itemName = QStringLiteral("file");
    const QString childName = QStringLiteral("string");
    const int iconsCount = emoticonElements.size();
    emoticons.clear();
    emoticonToPath.clear();
    cachedIcon.clear();

    for (int i = 0; i < iconsCount; ++i) {
        const QDomNode node = emoticonElements.at(i);
        const QString iconName = node.attributes().namedItem(itemName).nodeValue();
        const QString iconPath = QDir{path}.filePath(iconName);
        QDomElement stringElement = node.firstChildElement(childName);
        QStringList emoticonList;
        while (!stringElement.isNull()) {
            const QString emoticon = singleCharAsciiEmoticon(
                stringElement.text().replace("<", "&lt;").replace(">", "&gt;"));
            emoticonToPath.insert(emoticon, iconPath);
            emoticonList.append(emoticon);
            stringElement = stringElement.nextSibling().toElement();
        }

        emoticons.append(emoticonList);
    }

    constructRegex();
}

/**
 * @brief Creates the regex for replacing emoticons with the path to their pictures
 */
void SmileyPack::constructRegex()
{
    QString allPattern = QStringLiteral("(");
    QString regularPatterns;
    QString multiCharacterEmojiPatterns;

    // construct one big regex that matches on every emoticon
    for (const QString& emote : emoticonToPath.keys()) {
        if (!isAscii(emote)) {
            if (emote.toUcs4().length() == 1) {
                regularPatterns.append(emote);
                regularPatterns.append(QStringLiteral("|"));
            } else {
                multiCharacterEmojiPatterns.append(emote);
                multiCharacterEmojiPatterns.append(QStringLiteral("|"));
            }
        } else {
            // patterns like ":)" or ":smile:", don't match inside a word or else will hit punctuation and html tags
            regularPatterns.append(QStringLiteral(R"((?<=^|\s))") % QRegularExpression::escape(emote)
                                   % QStringLiteral(R"((?=$|\s))"));
            regularPatterns.append(QStringLiteral("|"));
        }
    }

    // Regexps are evaluated from left to right, insert multichar emojis first so they are evaluated first
    allPattern.append(multiCharacterEmojiPatterns);
    allPattern.append(regularPatterns);
    allPattern[allPattern.size() - 1] = QChar(')');

    // compile and optimize regex
    smilify.setPattern(allPattern);
    smilify.optimize();
}

/**
 * @brief Replaces all found text emoticons to HTML reference with its according icon filename
 * @param msg Message where to search for emoticons
 * @return Formatted copy of message
 */
QString SmileyPack::smileyfied(const QString& msg)
{
    const QMutexLocker<QMutex> locker(&loadingMutex);
    QString result(msg);

    int replaceDiff = 0;
    for (const auto& match : smilify.globalMatch(result)) {
        const int startPos = match.capturedStart();
        const int keyLength = match.capturedLength();
        const QString imgRichText = SmileyPack::getAsRichText(match.captured());
        result.replace(startPos + replaceDiff, keyLength, imgRichText);
        replaceDiff += imgRichText.length() - keyLength;
    }

    return result;
}

/**
 * @brief Returns all emoticons that was extracted from files, grouped by according icon file
 */
QList<QStringList> SmileyPack::getEmoticons() const
{
    const QMutexLocker<QMutex> locker(&loadingMutex);
    return emoticons;
}

/**
 * @brief Gets icon according to passed emoticon
 * @param emoticon Passed emoticon
 * @return Returns cached icon according to passed emoticon, null if no icon mapped to this emoticon
 */
std::shared_ptr<QIcon> SmileyPack::getAsIcon(const QString& emoticon) const
{
    const QMutexLocker<QMutex> locker(&loadingMutex);
    if (cachedIcon.contains(emoticon)) {
        return cachedIcon[emoticon];
    }

    const auto iconPathIt = emoticonToPath.find(emoticon);
    if (iconPathIt == emoticonToPath.end()) {
        return std::make_shared<QIcon>();
    }

    const QString& iconPath = iconPathIt.value();
    auto icon = std::make_shared<QIcon>(iconPath);
    cachedIcon[emoticon] = icon;
    return icon;
}
