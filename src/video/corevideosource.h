/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */


#pragma once

#include "videosource.h"

#include <QMutex>

#include <atomic>
#include <vpx/vpx_image.h>

class CoreVideoSource : public VideoSource
{
    Q_OBJECT
public:
    // VideoSource interface
    void subscribe() override;
    void unsubscribe() override;

private:
    CoreVideoSource();

    void pushFrame(const vpx_image_t* frame);
    void setDeleteOnClose(bool newstate);

    void stopSource();
    void restartSource();

private:
    std::atomic_int subscribers;
    std::atomic_bool deleteOnClose;
    QMutex biglock;
    std::atomic_bool stopped;

    friend class CoreAV;
    friend class ToxFriendCall;
};
