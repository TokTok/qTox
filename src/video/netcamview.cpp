/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "netcamview.h"

#include "camerasource.h"

#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/video/videosurface.h"
#include "src/widget/style.h"
#include "src/widget/tool/movablewidget.h"

#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QScreen>

#include <utility>

namespace {
const auto BTN_STATE_NONE = QVariant("none");
const auto BTN_STATE_RED = QVariant("red");
const int BTN_PANEL_HEIGHT = 55;
const int BTN_PANEL_WIDTH = 250;
const auto BTN_STYLE_SHEET_PATH = QStringLiteral("chatForm/fullScreenButtons.qss");
} // namespace

NetCamView::NetCamView(ToxPk friendPk_, CameraSource& cameraSource_, Settings& settings_,
                       Style& style_, Profile& profile, QWidget* parent)
    : QWidget(parent)
    , selfFrame{nullptr}
    , friendPk{std::move(friendPk_)}
    , e(false)
    , cameraSource{cameraSource_}
    , settings{settings_}
    , style{style_}
{
    verLayout = new QVBoxLayout(this);
    setWindowTitle(tr("Tox video"));

    buttonLayout = new QHBoxLayout();

    toggleMessagesButton = new QPushButton();
    enterFullScreenButton = new QPushButton();
    enterFullScreenButton->setText(tr("Full Screen"));

    buttonLayout->addStretch();
    buttonLayout->addWidget(toggleMessagesButton);
    buttonLayout->addWidget(enterFullScreenButton);

    connect(toggleMessagesButton, &QPushButton::clicked, this, &NetCamView::showMessageClicked);
    connect(enterFullScreenButton, &QPushButton::clicked, this, &NetCamView::toggleFullScreen);

    verLayout->addLayout(buttonLayout);
    verLayout->setContentsMargins(0, 0, 0, 0);

    setShowMessages(false);

    setStyleSheet("NetCamView { background-color: #c1c1c1; }");
    buttonPanel = new QFrame(this);
    buttonPanel->setStyleSheet(style.getStylesheet(BTN_STYLE_SHEET_PATH, settings));
    buttonPanel->setGeometry(0, 0, BTN_PANEL_WIDTH, BTN_PANEL_HEIGHT);

    auto* buttonPanelLayout = new QHBoxLayout(buttonPanel);
    buttonPanelLayout->setContentsMargins(20, 0, 20, 0);

    videoPreviewButton = createButton("videoPreviewButton", "none");
    videoPreviewButton->setToolTip(tr("Toggle video preview"));

    volumeButton = createButton("volButtonFullScreen", "none");
    volumeButton->setToolTip(tr("Mute audio"));

    microphoneButton = createButton("micButtonFullScreen", "none");
    microphoneButton->setToolTip(tr("Mute microphone"));

    endVideoButton = createButton("videoButtonFullScreen", "none");
    endVideoButton->setToolTip(tr("End video call"));

    exitFullScreenButton = createButton("exitFullScreenButton", "none");
    exitFullScreenButton->setToolTip(tr("Exit full screen"));

    connect(videoPreviewButton, &QPushButton::clicked, this, &NetCamView::toggleVideoPreview);
    connect(volumeButton, &QPushButton::clicked, this, &NetCamView::volMuteToggle);
    connect(microphoneButton, &QPushButton::clicked, this, &NetCamView::micMuteToggle);
    connect(endVideoButton, &QPushButton::clicked, this, &NetCamView::endVideoCall);
    connect(exitFullScreenButton, &QPushButton::clicked, this, &NetCamView::toggleFullScreen);

    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(videoPreviewButton);
    buttonPanelLayout->addWidget(volumeButton);
    buttonPanelLayout->addWidget(microphoneButton);
    buttonPanelLayout->addWidget(endVideoButton);
    buttonPanelLayout->addWidget(exitFullScreenButton);
    buttonPanelLayout->addStretch();

    videoSurface = new VideoSurface(profile.loadAvatar(friendPk), this);
    videoSurface->setMinimumHeight(256);

    verLayout->insertWidget(0, videoSurface, 1);

    selfVideoSurface = new VideoSurface(profile.loadAvatar(), this, true);
    selfVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    selfVideoSurface->setMouseTracking(true);
    selfVideoSurface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    selfFrame = new MovableWidget(videoSurface);
    selfFrame->show();

    auto* frameLayout = new QHBoxLayout(selfFrame);
    frameLayout->addWidget(selfVideoSurface);
    frameLayout->setContentsMargins(0, 0, 0, 0);

    updateRatio();
    connections +=
        connect(selfVideoSurface, &VideoSurface::ratioChanged, this, &NetCamView::updateRatio);

    connections += connect(videoSurface, &VideoSurface::boundaryChanged, this, [this]() {
        const QRect boundingRect = videoSurface->getBoundingRect();
        updateFrameSize(boundingRect.size());
        selfFrame->setBoundary(boundingRect);
    });

    connections += connect(videoSurface, &VideoSurface::ratioChanged, this, [this]() {
        selfFrame->setMinimumWidth(selfFrame->minimumHeight() * selfVideoSurface->getRatio());
        const QRect boundingRect = videoSurface->getBoundingRect();
        updateFrameSize(boundingRect.size());
        selfFrame->resetBoundary(boundingRect);
    });

    connections += connect(&profile, &Profile::selfAvatarChanged, this,
                           [this](const QPixmap& pixmap) { selfVideoSurface->setAvatar(pixmap); });

    connections += connect(&profile, &Profile::friendAvatarChanged, this,
                           [this](ToxPk friendPkArg, const QPixmap& pixmap) {
                               if (friendPk == friendPkArg)
                                   videoSurface->setAvatar(pixmap);
                           });

    const QRect videoSize = settings.getCamVideoRes();
    qDebug() << "SIZER" << videoSize;
}

NetCamView::~NetCamView()
{
    for (const QMetaObject::Connection& conn : connections)
        disconnect(conn);
}

void NetCamView::show(VideoSource* source, const QString& title)
{
    setSource(source);
    selfVideoSurface->setSource(&cameraSource);

    setTitle(title);
    QWidget::show();
}

void NetCamView::hide()
{
    setSource(nullptr);
    selfVideoSurface->setSource(nullptr);

    if (selfFrame != nullptr)
        selfFrame->deleteLater();

    selfFrame = nullptr;

    QWidget::hide();
}

void NetCamView::setSource(VideoSource* s)
{
    videoSurface->setSource(s);
}

void NetCamView::setTitle(const QString& title)
{
    setWindowTitle(title);
}

void NetCamView::showEvent(QShowEvent* event)
{
    std::ignore = event;
    selfFrame->resetBoundary(videoSurface->getBoundingRect());
}

void NetCamView::updateRatio()
{
    selfFrame->setMinimumWidth(selfFrame->minimumHeight() * selfVideoSurface->getRatio());
    selfFrame->setRatio(selfVideoSurface->getRatio());
}

void NetCamView::updateFrameSize(QSize size)
{
    selfFrame->setMaximumSize(size.height() / 3, size.width() / 3);

    if (selfFrame->maximumWidth() > selfFrame->maximumHeight())
        selfFrame->setMaximumWidth(selfFrame->maximumHeight() * selfVideoSurface->getRatio());
    else
        selfFrame->setMaximumHeight(selfFrame->maximumWidth() / selfVideoSurface->getRatio());
}

QSize NetCamView::getSurfaceMinSize()
{
    const QSize surfaceSize = videoSurface->minimumSize();
    const QSize buttonSize = toggleMessagesButton->size();
    const QSize panelSize(0, 45);

    return surfaceSize + buttonSize + panelSize;
}

void NetCamView::setShowMessages(bool show, bool notify)
{
    if (!show) {
        toggleMessagesButton->setText(tr("Hide messages"));
        toggleMessagesButton->setIcon(QIcon());
        return;
    }

    toggleMessagesButton->setText(tr("Show messages"));

    if (notify) {
        toggleMessagesButton->setIcon(QIcon(style.getImagePath("chatArea/info.svg", settings)));
    }
}

void NetCamView::toggleFullScreen()
{
    if (isFullScreen()) {
        exitFullScreen();
    } else {
        enterFullScreen();
    }
}

void NetCamView::enterFullScreen()
{
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    showFullScreen();
    enterFullScreenButton->hide();
    toggleMessagesButton->hide();
    const auto screenSize = QGuiApplication::screenAt(pos())->geometry();
    buttonPanel->setGeometry((screenSize.width() / 2) - buttonPanel->width() / 2,
                             screenSize.height() - BTN_PANEL_HEIGHT - 25, BTN_PANEL_WIDTH,
                             BTN_PANEL_HEIGHT);
    buttonPanel->show();
    buttonPanel->activateWindow();
    buttonPanel->raise();
}

void NetCamView::exitFullScreen()
{
    setWindowFlags(Qt::Widget);
    showNormal();
    buttonPanel->hide();
    enterFullScreenButton->show();
    toggleMessagesButton->show();
}

void NetCamView::endVideoCall()
{
    toggleFullScreen();
    emit videoCallEnd();
}

void NetCamView::toggleVideoPreview()
{
    toggleButtonState(videoPreviewButton);
    if (selfFrame->isHidden()) {
        selfFrame->show();
    } else {
        selfFrame->hide();
    }
}

QPushButton* NetCamView::createButton(const QString& name, const QString& state)
{
    auto* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", QVariant(state));
    btn->setStyleSheet(style.getStylesheet(BTN_STYLE_SHEET_PATH, settings));

    return btn;
}

void NetCamView::updateMuteVolButton(bool isMuted)
{
    updateButtonState(volumeButton, !isMuted);
}

void NetCamView::updateMuteMicButton(bool isMuted)
{
    updateButtonState(microphoneButton, !isMuted);
}

void NetCamView::toggleButtonState(QPushButton* btn)
{
    if (btn->property("state") == BTN_STATE_RED) {
        btn->setProperty("state", BTN_STATE_NONE);
    } else {
        btn->setProperty("state", BTN_STATE_RED);
    }

    btn->setStyleSheet(style.getStylesheet(BTN_STYLE_SHEET_PATH, settings));
}

void NetCamView::updateButtonState(QPushButton* btn, bool active)
{
    if (active) {
        btn->setProperty("state", BTN_STATE_NONE);
    } else {
        btn->setProperty("state", BTN_STATE_RED);
    }

    btn->setStyleSheet(style.getStylesheet(BTN_STYLE_SHEET_PATH, settings));
}

void NetCamView::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();
    if (key == Qt::Key_Escape && isFullScreen()) {
        exitFullScreen();
    }
}

void NetCamView::closeEvent(QCloseEvent* event)
{
    exitFullScreen();
    event->ignore();
}
