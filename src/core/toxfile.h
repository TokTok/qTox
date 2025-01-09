/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/core/toxfilepause.h"
#include "src/core/toxfileprogress.h"

#include <QCryptographicHash>
#include <QString>

#include <memory>

class QFile;
class QTimer;

struct ToxFile
{
    // Note do not change values, these are directly inserted into the DB in their
    // current form, changing order would mess up database state!
    enum FileStatus
    {
        INITIALIZING = 0,
        PAUSED = 1,
        TRANSMITTING = 2,
        BROKEN = 3,
        CANCELED = 4,
        FINISHED = 5,
    };

    // Note do not change values, these are directly inserted into the DB in their
    // current form (can add fields though as db representation is an int)
    enum FileDirection : bool
    {
        SENDING = false,
        RECEIVING = true,
    };

    ToxFile();
    ToxFile(uint32_t fileNum_, uint32_t friendId_, QString fileName_, QString filePath_,
            uint64_t filesize, FileDirection direction);

    bool operator==(const ToxFile& other) const;
    bool operator!=(const ToxFile& other) const;

    void setFilePath(QString path);
    bool open(bool write);

    uint8_t fileKind;
    uint32_t fileNum;
    uint32_t friendId;
    QString fileName;
    QString filePath;
    std::shared_ptr<QFile> file;
    FileStatus status;
    FileDirection direction;
    QByteArray avatarData;
    QByteArray resumeFileId;
    std::shared_ptr<QCryptographicHash> hashGenerator =
        std::make_shared<QCryptographicHash>(QCryptographicHash::Sha256);
    ToxFilePause pauseStatus;
    ToxFileProgress progress;
};
