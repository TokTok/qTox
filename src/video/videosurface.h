/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/video/videosource.h"

#include <QWidget>

#include <atomic>
#include <memory>

class VideoSurface : public QWidget
{
    Q_OBJECT

public:
    explicit VideoSurface(QPixmap avatar_, QWidget* parent = nullptr, bool expanding_ = false);
    VideoSurface(const QPixmap& avatar_, VideoSource* source_, QWidget* parent = nullptr);
    ~VideoSurface() override;

    bool isExpanding() const;
    void setSource(VideoSource* src);
    QRect getBoundingRect() const;
    float getRatio() const;
    void setAvatar(const QPixmap& pixmap);
    QPixmap getAvatar() const;

signals:
    void ratioChanged();
    void boundaryChanged();

protected:
    void subscribe();
    void unsubscribe();

    void paintEvent(QPaintEvent* event) final;
    void resizeEvent(QResizeEvent* event) final;
    void showEvent(QShowEvent* event) final;

private slots:
    void onNewFrameAvailable(const std::shared_ptr<VideoFrame>& newFrame);
    void onSourceStopped();

private:
    void recalculateBounds();
    void lock();
    void unlock();

    QRect boundingRect;
    VideoSource* source;
    std::shared_ptr<VideoFrame> lastFrame;
    std::atomic_bool frameLock;
    uint8_t hasSubscribed;
    QPixmap avatar;
    float ratio;
    bool expanding;
};
