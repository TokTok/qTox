/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/core/toxpk.h"

#include <QVector>
#include <QWidget>

class QHBoxLayout;
struct vpx_image;
class VideoSource;
class QFrame;
class MovableWidget;
class QVBoxLayout;
class VideoSurface;
class QPushButton;
class QKeyEvent;
class QCloseEvent;
class QShowEvent;
class CameraSource;
class Settings;
class Style;
class Profile;

class NetCamView : public QWidget
{
    Q_OBJECT

public:
    NetCamView(ToxPk friendPk_, CameraSource& cameraSource, Settings& settings, Style& style,
               Profile& profile, QWidget* parent = nullptr);
    ~NetCamView() override;

    virtual void show(VideoSource* source, const QString& title);
    virtual void hide();

    void setSource(VideoSource* s);
    void setTitle(const QString& title);
    QSize getSurfaceMinSize();

protected:
    void showEvent(QShowEvent* event) override;
    QVBoxLayout* verLayout;
    VideoSurface* videoSurface;
    QPushButton* enterFullScreenButton = nullptr;

signals:
    void showMessageClicked();
    void videoCallEnd();
    void volMuteToggle();
    void micMuteToggle();

public slots:
    void setShowMessages(bool show, bool notify = false);
    void updateMuteVolButton(bool isMuted);
    void updateMuteMicButton(bool isMuted);

private slots:
    void updateRatio();

private:
    void updateFrameSize(QSize size);
    QPushButton* createButton(const QString& name, const QString& state);
    void toggleFullScreen();
    void enterFullScreen();
    void exitFullScreen();
    void endVideoCall();
    void toggleVideoPreview();
    void toggleButtonState(QPushButton* btn);
    void updateButtonState(QPushButton* btn, bool active);
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    VideoSurface* selfVideoSurface;
    MovableWidget* selfFrame;
    ToxPk friendPk;
    bool e;
    QVector<QMetaObject::Connection> connections;
    QHBoxLayout* buttonLayout = nullptr;
    QPushButton* toggleMessagesButton = nullptr;
    QFrame* buttonPanel = nullptr;
    QPushButton* videoPreviewButton = nullptr;
    QPushButton* volumeButton = nullptr;
    QPushButton* microphoneButton = nullptr;
    QPushButton* endVideoButton = nullptr;
    QPushButton* exitFullScreenButton = nullptr;
    CameraSource& cameraSource;
    Settings& settings;
    Style& style;
};
