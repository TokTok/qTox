/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#include "src/core/toxfile.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include <tox/tox.h>
#include <utility>

#define TOX_HEX_ID_LENGTH 2 * TOX_ADDRESS_SIZE

/**
 * @file corestructs.h
 * @brief Some headers use Core structs but don't need to include all of core.h
 *
 * They should include this file directly instead to reduce compilation times
 *
 * @var uint8_t ToxFile::fileKind
 * @brief Data file (default) or avatar
 */

ToxFile::ToxFile()
    : fileKind(0)
    , fileNum(0)
    , friendId(0)
    , status(INITIALIZING)
    , direction(SENDING)
    , progress(0)
{
}

/**
 * @brief ToxFile constructor
 */
ToxFile::ToxFile(uint32_t fileNum_, uint32_t friendId_, QString fileName_, QString filePath_,
                 uint64_t filesize, FileDirection direction_)
    : fileKind{TOX_FILE_KIND_DATA}
    , fileNum(fileNum_)
    , friendId(friendId_)
    , fileName{std::move(fileName_)}
    , filePath{filePath_}
    , file{new QFile(filePath_)}
    , status{INITIALIZING}
    , direction{direction_}
    , progress(filesize)
{
}

bool ToxFile::operator==(const ToxFile& other) const
{
    return (fileNum == other.fileNum) && (friendId == other.friendId)
           && (direction == other.direction);
}

bool ToxFile::operator!=(const ToxFile& other) const
{
    return !(*this == other);
}

uint64_t ToxFile::partialSizeOnDisk(const QString& path, uint64_t expectedSize)
{
    const QFileInfo fi(path);
    if (!fi.exists() || !fi.isFile()) {
        return 0;
    }
    const auto size = static_cast<uint64_t>(fi.size());
    return (size > 0 && size < expectedSize) ? size : 0;
}

void ToxFile::setFilePath(QString path)
{
    filePath = path;
    file->setFileName(path);
}

bool ToxFile::open(bool write, bool resume)
{
    if (write) {
        if (resume) {
            // Append always writes to EOF (seek is ignored); this is intentional
            // for resume so new chunks are appended after the partial data.
            return file->open(QIODevice::WriteOnly | QIODevice::Append);
        }
        // ReadWrite does not truncate; the caller is expected to have removed
        // the existing file (tryRemoveFile) or to seek before writing.
        return file->open(QIODevice::ReadWrite);
    }
    return file->open(QIODevice::ReadOnly);
}
