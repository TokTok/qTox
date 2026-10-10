/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */


#pragma once

#include "toxfile.h"

#include "src/core/toxpk.h"
#include "src/model/status.h"

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QString>

#include <cstdint>
#include <memory>
#include <tox/tox.h> // Tox_File_Control

struct Tox;
struct Tox_Event_File_Recv;
struct Tox_Event_File_Recv_Control;
struct Tox_Event_File_Chunk_Request;
struct Tox_Event_File_Recv_Chunk;
struct Tox_Dispatch;
class Core;
class CoreFile;

using CoreFilePtr = std::unique_ptr<CoreFile>;

class CoreFile : public QObject
{
    Q_OBJECT
    friend class TestFileTransferWidget;

    friend class Core;

public:
    void handleAvatarOffer(uint32_t friendId, uint32_t fileId, bool accept, uint64_t filesize);
    static CoreFilePtr makeCoreFile(Core* core, Tox* tox, QRecursiveMutex& coreLoopLock);

    void sendFile(uint32_t friendId, QString filename, QString filePath, long long filesize);
    void sendAvatarFile(uint32_t friendId, const QByteArray& data);
    void pauseResumeFile(uint32_t friendId, uint32_t fileId);
    void cancelFileSend(uint32_t friendId, uint32_t fileId);

    void cancelFileRecv(uint32_t friendId, uint32_t fileId);
    void rejectFileRecvRequest(uint32_t friendId, uint32_t fileId);
    void acceptFileRecvRequest(uint32_t friendId, uint32_t fileId, QString path);

    unsigned corefileIterationInterval();

signals:
    void fileSendStarted(ToxFile file);
    void fileReceiveRequested(ToxFile file);
    void fileTransferAccepted(ToxFile file);
    void fileTransferCancelled(ToxFile file);
    void fileTransferFinished(ToxFile file);
    void fileTransferPaused(ToxFile file);
    void fileTransferInfo(ToxFile file);
    void fileTransferRemotePausedUnpaused(ToxFile file, bool paused);
    void fileTransferBrokenUnbroken(ToxFile file, bool broken);
    void fileNameChanged(const ToxPk& friendPk);
    void fileSendFailed(uint32_t friendId, const QString& fname);

private:
    CoreFile(Tox* core_, QRecursiveMutex& coreLoopLock_);

    ToxFile* findFile(uint32_t friendId, uint32_t fileId);
    void addFile(uint32_t friendId, uint32_t fileId, const ToxFile& file);
    void removeFile(uint32_t friendId, uint32_t fileId);
    static constexpr uint64_t getFriendKey(uint32_t friendId, uint32_t fileId)
    {
        return (static_cast<std::uint64_t>(friendId) << 32) + fileId;
    }

    static void onFileRecv(const Tox_Event_File_Recv* event, Core* core);
    static void onFileRecvControl(const Tox_Event_File_Recv_Control* event, Core* core);
    static void onFileChunkRequest(const Tox_Event_File_Chunk_Request* event, Core* core);
    static void onFileRecvChunk(const Tox_Event_File_Recv_Chunk* event, Core* core);

public:
    static QString getCleanFileName(QString filename);

private slots:
    void onConnectionStatusChanged(uint32_t friendId, Status::Status state);

private:
    QHash<uint64_t, ToxFile> fileMap;
    Tox* tox;
    QRecursiveMutex* coreLoopLock = nullptr;
};
