/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "videosurface.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include "src/video/videoframe.h"
#include "src/widget/friendwidget.h"
#include "src/widget/style.h"

#include <QDebug>
#include <QLabel>
#include <QPainter>
#include <utility>

namespace {
float getSizeRatio(const QSize size)
{
    return size.width() / static_cast<float>(size.height());
}
} // namespace

/**
 * @var std::atomic_bool VideoSurface::frameLock
 * @brief Fast lock for lastFrame.
 */
VideoSurface::VideoSurface(QPixmap avatar_, QWidget* parent, bool expanding_)
    : QWidget{parent}
    , source{nullptr}
    , frameLock{false}
    , hasSubscribed{0}
    , avatar{std::move(avatar_)}
    , ratio{1.0f}
    , expanding{expanding_}
{
    recalculateBounds();
}

VideoSurface::VideoSurface(const QPixmap& avatar_, VideoSource* source_, QWidget* parent)
    : VideoSurface(avatar_, parent)
{
    setSource(source_);
}

VideoSurface::~VideoSurface()
{
    unsubscribe();
}

bool VideoSurface::isExpanding() const
{
    return expanding;
}

/**
 * @brief Update source.
 * @note nullptr is a valid option.
 * @param src source to set.
 *
 * Unsubscribe from old source and subscribe to new.
 */
void VideoSurface::setSource(VideoSource* src)
{
    if (source == src)
        return;

    unsubscribe();
    source = src;
    subscribe();
}

QRect VideoSurface::getBoundingRect() const
{
    QRect bRect = boundingRect;
    bRect.setBottomRight(QPoint(boundingRect.bottom() + 1, boundingRect.right() + 1));
    return boundingRect;
}

float VideoSurface::getRatio() const
{
    return ratio;
}

void VideoSurface::setAvatar(const QPixmap& pixmap)
{
    avatar = pixmap;
    update();
}

QPixmap VideoSurface::getAvatar() const
{
    return avatar;
}

void VideoSurface::subscribe()
{
    if ((source != nullptr) && hasSubscribed++ == 0) {
        source->subscribe();
        connect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
        connect(source, &VideoSource::sourceStopped, this, &VideoSurface::onSourceStopped);
    }
}

void VideoSurface::unsubscribe()
{
    if ((source == nullptr) || hasSubscribed == 0)
        return;

    if (--hasSubscribed != 0)
        return;

    lock();
    lastFrame.reset();
    unlock();

    ratio = 1.0f;
    recalculateBounds();
    emit ratioChanged();
    emit boundaryChanged();

    disconnect(source, &VideoSource::frameAvailable, this, &VideoSurface::onNewFrameAvailable);
    disconnect(source, &VideoSource::sourceStopped, this, &VideoSurface::onSourceStopped);
    source->unsubscribe();
}

void VideoSurface::onNewFrameAvailable(const std::shared_ptr<VideoFrame>& newFrame)
{
    QSize newSize;

    lock();
    lastFrame = newFrame;
    newSize = lastFrame->getSourceDimensions().size();
    unlock();

    const float newRatio = getSizeRatio(newSize);

    if (!qFuzzyCompare(newRatio, ratio) && isVisible()) {
        ratio = newRatio;
        recalculateBounds();
        emit ratioChanged();
        emit boundaryChanged();
    }

    update();
}

void VideoSurface::onSourceStopped()
{
    // If the source's stream is on hold, just revert back to the avatar view
    lastFrame.reset();
    update();
}

void VideoSurface::paintEvent(QPaintEvent* event)
{
    std::ignore = event;
    lock();

    QPainter painter(this);
    painter.fillRect(painter.viewport(), Qt::black);
    if (lastFrame) {
        const QImage frame = lastFrame->toQImage(rect().size());
        if (frame.isNull())
            lastFrame.reset();
        painter.drawImage(boundingRect, frame, frame.rect(), Qt::NoFormatConversion);
    } else {
        painter.fillRect(boundingRect, Qt::white);
        QPixmap drawnAvatar = avatar;

        if (drawnAvatar.isNull())
            drawnAvatar = Style::scaleSvgImage(":/img/contact_dark.svg", boundingRect.width(),
                                               boundingRect.height());

        painter.drawPixmap(boundingRect, drawnAvatar, drawnAvatar.rect());
    }

    unlock();
}

void VideoSurface::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    recalculateBounds();
    emit boundaryChanged();
}

void VideoSurface::showEvent(QShowEvent* e)
{
    std::ignore = e;
    // emit ratioChanged();
}

void VideoSurface::recalculateBounds()
{
    if (expanding) {
        boundingRect = contentsRect();
    } else {
        QPoint pos;
        QSize size;
        const QSize usableSize = contentsRect().size();
        const int possibleWidth = usableSize.height() * ratio;

        if (possibleWidth > usableSize.width())
            size = (QSize(usableSize.width(), usableSize.width() / ratio));
        else
            size = (QSize(possibleWidth, usableSize.height()));

        pos.setX(width() / 2 - size.width() / 2);
        pos.setY(height() / 2 - size.height() / 2);
        boundingRect.setRect(pos.x(), pos.y(), size.width(), size.height());
    }

    update();
}

void VideoSurface::lock()
{
    // Fast lock
    bool expected = false;
    while (!frameLock.compare_exchange_weak(expected, true))
        expected = false;
}

void VideoSurface::unlock()
{
    frameLock = false;
}
