/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "avform.h"

#include "audio/audio.h"
#include "audio/iaudiosettings.h"
#include "audio/iaudiosource.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/video/cameradevice.h"
#include "src/video/camerasource.h"
#include "src/video/ivideosettings.h"
#include "src/video/videosurface.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "src/widget/tool/screenshotgrabber.h"
#include "src/widget/translator.h"
#include "util/ranges.h"

#include <QDebug>
#include <QScreen>
#include <QShowEvent>

#include <cassert>
#include <map>

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER ALC_DEVICE_SPECIFIER
#endif

AVForm::AVForm(IAudioControl& audio_, CoreAV* coreAV_, CameraSource& camera_,
               IAudioSettings* audioSettings_, IVideoSettings* videoSettings_, Style& style)
    : GenericForm(QPixmap(":/img/settings/av.png"), style)
    , audio(audio_)
    , coreAV{coreAV_}
    , audioSettings{audioSettings_}
    , videoSettings{videoSettings_}
    , camVideoSurface(nullptr)
    , camera(camera_)
{
    setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    cbEnableTestSound->setChecked(audioSettings_->getEnableTestSound());
    cbEnableTestSound->setToolTip(tr("Play a test sound while changing the output volume."));

    connect(rescanButton, &QPushButton::clicked, this, &AVForm::rescanDevices);

    playbackSlider->setTracking(false);
    playbackSlider->setMaximum(totalSliderSteps);
    playbackSlider->setValue(getStepsFromValue(audioSettings_->getOutVolume(),
                                               audioSettings_->getOutVolumeMin(),
                                               audioSettings_->getOutVolumeMax()));
    playbackSlider->installEventFilter(this);

    microphoneSlider->setToolTip(tr("Use slider to set the gain of your input device ranging"
                                    " from %1dB to %2dB.")
                                     .arg(audio_.minInputGain())
                                     .arg(audio_.maxInputGain()));
    microphoneSlider->setMaximum(totalSliderSteps);
    microphoneSlider->setTickPosition(QSlider::TicksBothSides);
    static const int numTicks = 4;
    microphoneSlider->setTickInterval(totalSliderSteps / numTicks);
    microphoneSlider->setTracking(false);
    microphoneSlider->installEventFilter(this);
    microphoneSlider->setValue(getStepsFromValue(audioSettings_->getAudioInGainDecibel(),
                                                 audio_.minInputGain(), audio_.maxInputGain()));

    audioThresholdSlider->setToolTip(tr("Use slider to set the activation volume for your"
                                        " input device."));
    audioThresholdSlider->setMaximum(totalSliderSteps);
    audioThresholdSlider->setValue(getStepsFromValue(audioSettings_->getAudioThreshold(),
                                                     audio_.minInputThreshold(),
                                                     audio_.maxInputThreshold()));
    audioThresholdSlider->setTracking(false);
    audioThresholdSlider->installEventFilter(this);

    volumeDisplay->setMaximum(totalSliderSteps);

    fillAudioQualityComboBox();

    eventsInit();

    for (QScreen* qScreen : QGuiApplication::screens()) {
        connect(qScreen, &QScreen::geometryChanged, this, &AVForm::rescanDevices);
    }
    auto* qGUIApp = qobject_cast<QGuiApplication*>(qApp);
    assert(qGUIApp);
    connect(qGUIApp, &QGuiApplication::screenAdded, this, &AVForm::trackNewScreenGeometry);
    connect(qGUIApp, &QGuiApplication::screenAdded, this, &AVForm::rescanDevices);
    connect(qGUIApp, &QGuiApplication::screenRemoved, this, &AVForm::rescanDevices);
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

AVForm::~AVForm()
{
    killVideoSurface();
    Translator::unregister(this);
}

void AVForm::hideEvent(QHideEvent* event)
{
    audioSink.reset();
    audioSrc.reset();

    if (camVideoSurface != nullptr) {
        camVideoSurface->setSource(nullptr);
        killVideoSurface();
    }
    videoDeviceList.clear();

    GenericForm::hideEvent(event);
}

void AVForm::showEvent(QShowEvent* event)
{
    getAudioOutDevices();
    getAudioInDevices();
    createVideoSurface();
    getVideoDevices();

    if (audioSrc == nullptr) {
        audioSrc = audio.makeSource();
        if (audioSrc != nullptr) {
            connect(audioSrc.get(), &IAudioSource::volumeAvailable, this, &AVForm::setVolume);
        }
    }

    if (audioSink == nullptr) {
        audioSink = audio.makeSink();
    }

    GenericForm::showEvent(event);
}

void AVForm::open(const QString& devName, const VideoMode& mode)
{
    const QRect rect = mode.toRect();
    videoSettings->setCamVideoRes(rect);
    videoSettings->setCamVideoFPS(mode.fps);
    camera.setupDevice(devName, mode);
}

void AVForm::trackNewScreenGeometry(QScreen* qScreen) const
{
    connect(qScreen, &QScreen::geometryChanged, this, &AVForm::rescanDevices);
}

void AVForm::rescanDevices()
{
    getAudioInDevices();
    getAudioOutDevices();
    getVideoDevices();
}

void AVForm::setVolume(qreal value)
{
    volumeDisplay->setValue(getStepsFromValue(value, audio.minOutputVolume(), audio.maxOutputVolume()));
}

void AVForm::on_videoModesComboBox_currentIndexChanged(int index)
{
    assert(0 <= index && index < videoModes.size());
    const int devIndex = videoDevCombobox->currentIndex();
    assert(0 <= devIndex && devIndex < videoDeviceList.size());

    const QString devName = videoDeviceList[devIndex].first;
    const VideoMode newMode = videoModes[index];

    if (CameraDevice::isScreen(devName) && newMode == VideoMode()) {
        if (videoSettings->getScreenGrabbed()) {
            const VideoMode screenMode(videoSettings->getScreenRegion());
            open(devName, screenMode);
            return;
        }

        auto onGrabbed = [devName, this](QRect region) {
            VideoMode mode(region);
            mode.width = mode.width / 2 * 2;
            mode.height = mode.height / 2 * 2;

            // Needed, if the virtual screen origin is the top left corner of the primary screen
            const QRect screen = QApplication::primaryScreen()->virtualGeometry();
            mode.x += screen.x();
            mode.y += screen.y();

            videoSettings->setScreenRegion(mode.toRect());
            videoSettings->setScreenGrabbed(true);

            open(devName, mode);
        };

        // We're not using the platform-specific grabber here, because all we want is a rectangle
        // selection. We don't actually need a screenshot to be taken.
        //
        // Note: grabber is self-managed and will destroy itself when done.
        auto* screenshotGrabber = new ScreenshotGrabber(this);

        connect(screenshotGrabber, &ScreenshotGrabber::regionChosen, this, onGrabbed,
                Qt::QueuedConnection);
        screenshotGrabber->showGrabber();
        return;
    }

    videoSettings->setScreenGrabbed(false);
    open(devName, newMode);
}

void AVForm::selectBestModes(QVector<VideoMode>& allVideoModes)
{
    if (allVideoModes.isEmpty()) {
        qCritical() << "Trying to select best mode from empty modes list";
        return;
    }

    // Identify the best resolutions available for the supposed XXXXp resolutions.
    constexpr std::array<std::pair<int, VideoMode>, 8> idealModes{{
        {120, VideoMode(160, 120)},    // 1
        {240, VideoMode(430, 240)},    // 2
        {360, VideoMode(640, 360)},    // 3
        {480, VideoMode(854, 480)},    // 4
        {720, VideoMode(1280, 720)},   // 5
        {1080, VideoMode(1920, 1080)}, // 6
        {1440, VideoMode(2560, 1440)}, // 7
        {2160, VideoMode(3840, 2160)}, // 8
    }};

    std::map<int, int> bestModeIndices;
    for (int i = 0; i < allVideoModes.size(); ++i) {
        const VideoMode mode = allVideoModes[i];

        // PS3-Cam protection, everything above 60fps makes no sense
        if (mode.fps > 60)
            continue;

        for (const auto& [res, idealMode] : idealModes) {
            // don't take approximately correct resolutions unless they really
            // are close
            if (mode.norm(idealMode) > idealMode.tolerance())
                continue;

            if (!bestModeIndices.contains(res)) {
                bestModeIndices[res] = i;
                continue;
            }

            const VideoMode best = allVideoModes[bestModeIndices[res]];
            if (mode.norm(idealMode) < best.norm(idealMode)) {
                bestModeIndices[res] = i;
                continue;
            }

            if (mode.norm(idealMode) == best.norm(idealMode)) {
                // prefer higher FPS and "better" pixel formats
                if (mode.fps > best.fps) {
                    bestModeIndices[res] = i;
                    continue;
                }

                const bool better =
                    CameraDevice::betterPixelFormat(mode.pixel_format, best.pixel_format);
                if (mode.fps >= best.fps && better)
                    bestModeIndices[res] = i;
            }
        }
    }

    QVector<VideoMode> newVideoModes;
    for (const auto& [_, modeIndex] : qtox::views::reverse(bestModeIndices)) {
        const VideoMode mode_ = allVideoModes[modeIndex];

        if (newVideoModes.empty()) {
            newVideoModes.push_back(mode_);
        } else {
            const int size = getModeSize(mode_);
            auto result = std::find_if(newVideoModes.cbegin(), newVideoModes.cend(),
                                       [size](VideoMode mode) { return getModeSize(mode) == size; });

            if (result == newVideoModes.end())
                newVideoModes.push_back(mode_);
        }
    }
    allVideoModes = newVideoModes;
}

void AVForm::fillCameraModesComboBox()
{
    qDebug() << "selected Modes:";
    const bool previouslyBlocked = videoModesComboBox->blockSignals(true);
    videoModesComboBox->clear();

    for (auto mode : videoModes) {
        QString str;
        const std::string pixelFormat =
            CameraDevice::getPixelFormatString(mode.pixel_format).toStdString();
        qDebug("width: %d, height: %d, fps: %f, pixel format: %s", mode.width, mode.height,
               static_cast<double>(mode.fps), pixelFormat.c_str());

        if ((mode.height != 0) && (mode.width != 0)) {
            str += QString("%1p").arg(mode.height);
        } else {
            str += tr("Default resolution");
        }

        videoModesComboBox->addItem(str);
    }

    if (videoModes.isEmpty())
        videoModesComboBox->addItem(tr("Default resolution"));

    videoModesComboBox->blockSignals(previouslyBlocked);
}

int AVForm::searchPreferredIndex()
{
    const QRect prefRes = videoSettings->getCamVideoRes();
    const float prefFPS = videoSettings->getCamVideoFPS();

    for (int i = 0; i < videoModes.size(); ++i) {
        const VideoMode mode = videoModes[i];
        if (mode.width == prefRes.width() && mode.height == prefRes.height()
            && (qAbs(mode.fps - prefFPS) < 0.0001f)) {
            return i;
        }
    }

    return -1;
}

void AVForm::fillScreenModesComboBox()
{
    const bool previouslyBlocked = videoModesComboBox->blockSignals(true);
    videoModesComboBox->clear();

    for (int i = 0; i < videoModes.size(); ++i) {
        const VideoMode mode = videoModes[i];
        const std::string pixelFormat =
            CameraDevice::getPixelFormatString(mode.pixel_format).toStdString();
        qDebug("%dx%d+%d,%d fps: %f, pixel format: %s", mode.width, mode.height, mode.x, mode.y,
               static_cast<double>(mode.fps), pixelFormat.c_str());

        QString name;
        if ((mode.width != 0) && (mode.height != 0))
            name = tr("Screen %1").arg(i + 1);
        else
            name = tr("Select region");

        videoModesComboBox->addItem(name);
    }

    videoModesComboBox->blockSignals(previouslyBlocked);
}

void AVForm::fillAudioQualityComboBox()
{
    const bool previouslyBlocked = audioQualityComboBox->blockSignals(true);

    audioQualityComboBox->addItem(tr("High (64 kBps)"), 64);
    audioQualityComboBox->addItem(tr("Medium (32 kBps)"), 32);
    audioQualityComboBox->addItem(tr("Low (16 kBps)"), 16);
    audioQualityComboBox->addItem(tr("Very low (8 kBps)"), 8);

    const int currentBitrate = audioSettings->getAudioBitrate();
    const int index = audioQualityComboBox->findData(currentBitrate);

    audioQualityComboBox->setCurrentIndex(index);
    audioQualityComboBox->blockSignals(previouslyBlocked);
}

void AVForm::updateVideoModes(int curIndex)
{
    if (curIndex < 0 || curIndex >= videoDeviceList.size()) {
        qWarning() << "Invalid index:" << curIndex;
        return;
    }
    const QString devName = videoDeviceList[curIndex].first;
    QVector<VideoMode> allVideoModes = CameraDevice::getVideoModes(devName);

    qDebug("available Modes:");
    const bool isScreen = CameraDevice::isScreen(devName);
    if (isScreen) {
        // Add extra video mode to region selection
        allVideoModes.push_back(VideoMode());
        videoModes = allVideoModes;
        fillScreenModesComboBox();
    } else {
        selectBestModes(allVideoModes);
        videoModes = allVideoModes;
        fillCameraModesComboBox();
    }

    const int preferredIndex = searchPreferredIndex();
    if (preferredIndex != -1) {
        videoSettings->setScreenGrabbed(false);
        videoModesComboBox->blockSignals(true);
        videoModesComboBox->setCurrentIndex(preferredIndex);
        videoModesComboBox->blockSignals(false);
        open(devName, videoModes[preferredIndex]);
        return;
    }

    if (isScreen) {
        const QRect rect = videoSettings->getScreenRegion();
        const VideoMode mode(rect);

        videoSettings->setScreenGrabbed(true);
        videoModesComboBox->setCurrentIndex(videoModes.size() - 1);
        open(devName, mode);
        return;
    }

    // If the user hasn't set a preferred resolution yet,
    // we'll pick the resolution in the middle of the list,
    // and the best FPS for that resolution.
    // If we picked the lowest resolution, the quality would be awful
    // but if we picked the largest, FPS would be bad and thus quality bad too.
    const int mid = (videoModes.size() - 1) / 2;
    videoModesComboBox->setCurrentIndex(mid);
}

void AVForm::on_videoDevCombobox_currentIndexChanged(int index)
{
    assert(0 <= index && index < videoDeviceList.size());

    videoSettings->setScreenGrabbed(false);
    const QString dev = videoDeviceList[index].first;
    videoSettings->setVideoDev(dev);
    const bool previouslyBlocked = videoModesComboBox->blockSignals(true);
    updateVideoModes(index);
    videoModesComboBox->blockSignals(previouslyBlocked);

    if (videoSettings->getScreenGrabbed()) {
        return;
    }

    const int modeIndex = videoModesComboBox->currentIndex();
    VideoMode mode = VideoMode();
    if (0 <= modeIndex && modeIndex < videoModes.size()) {
        mode = videoModes[modeIndex];
    }

    camera.setupDevice(dev, mode);
    if (dev == "none") {
        coreAV->sendNoVideo();
    }
}

void AVForm::on_audioQualityComboBox_currentIndexChanged(int index)
{
    std::ignore = index;
    audioSettings->setAudioBitrate(audioQualityComboBox->currentData().toInt());
}

void AVForm::getVideoDevices()
{
    const QString settingsInDev = videoSettings->getVideoDev();
    int videoDevIndex = 0;
    videoDeviceList = CameraDevice::getDeviceList();
    // prevent currentIndexChanged to be fired while adding items
    videoDevCombobox->blockSignals(true);
    videoDevCombobox->clear();
    for (const QPair<QString, QString>& device : videoDeviceList) {
        videoDevCombobox->addItem(device.second);
        if (device.first == settingsInDev)
            videoDevIndex = videoDevCombobox->count() - 1;
    }
    videoDevCombobox->setCurrentIndex(videoDevIndex);
    videoDevCombobox->blockSignals(false);
    updateVideoModes(videoDevIndex);
}

int AVForm::getModeSize(VideoMode mode)
{
    return qRound(mode.height / 120.0) * 120;
}

void AVForm::getAudioInDevices()
{
    QStringList deviceNames;
    deviceNames << tr("Disabled") << audio.inDeviceNames();

    inDevCombobox->blockSignals(true);
    inDevCombobox->clear();
    inDevCombobox->addItems(deviceNames);
    inDevCombobox->blockSignals(false);

    int idx = 0;
    const bool enabled = audioSettings->getAudioInDevEnabled();
    if (enabled && deviceNames.size() > 1) {
        const QString dev = audioSettings->getInDev();
        idx = qMax(deviceNames.indexOf(dev), 1);
    }
    inDevCombobox->setCurrentIndex(idx);
}

void AVForm::getAudioOutDevices()
{
    QStringList deviceNames;
    deviceNames << tr("Disabled") << audio.outDeviceNames();

    outDevCombobox->blockSignals(true);
    outDevCombobox->clear();
    outDevCombobox->addItems(deviceNames);
    outDevCombobox->blockSignals(false);

    int idx = 0;
    const bool enabled = audioSettings->getAudioOutDevEnabled();
    if (enabled && deviceNames.size() > 1) {
        const QString dev = audioSettings->getOutDev();
        idx = qMax(deviceNames.indexOf(dev), 1);
    }
    outDevCombobox->setCurrentIndex(idx);
}

void AVForm::on_inDevCombobox_currentIndexChanged(int deviceIndex)
{
    const bool inputEnabled = deviceIndex > 0;
    audioSettings->setAudioInDevEnabled(inputEnabled);

    QString deviceName{};
    if (inputEnabled) {
        deviceName = inDevCombobox->itemText(deviceIndex);
    }

    const QString oldName = audioSettings->getInDev();
    if (oldName != deviceName) {
        audioSettings->setInDev(deviceName);
        audio.reinitInput(deviceName);
        audioSrc = audio.makeSource();
        if (audioSrc != nullptr) {
            connect(audioSrc.get(), &IAudioSource::volumeAvailable, this, &AVForm::setVolume);
        }
    }

    microphoneSlider->setEnabled(inputEnabled);
    if (!inputEnabled) {
        volumeDisplay->setValue(volumeDisplay->minimum());
    }
}

void AVForm::on_outDevCombobox_currentIndexChanged(int deviceIndex)
{
    const bool outputEnabled = deviceIndex > 0;
    audioSettings->setAudioOutDevEnabled(outputEnabled);

    QString deviceName{};
    if (outputEnabled) {
        deviceName = outDevCombobox->itemText(deviceIndex);
    }

    const QString oldName = audioSettings->getOutDev();

    if (oldName != deviceName) {
        audioSettings->setOutDev(deviceName);
        audio.reinitOutput(deviceName);
        audioSink = audio.makeSink();
    }

    playbackSlider->setEnabled(outputEnabled);
}

void AVForm::on_playbackSlider_valueChanged(int sliderSteps)
{
    const int settingsVolume = getValueFromSteps(sliderSteps, audioSettings->getOutVolumeMin(),
                                                 audioSettings->getOutVolumeMax());
    audioSettings->setOutVolume(settingsVolume);

    if (audio.isOutputReady()) {
        const qreal volume =
            getValueFromSteps(sliderSteps, audio.minOutputVolume(), audio.maxOutputVolume());
        audio.setOutputVolume(volume);

        if (cbEnableTestSound->isChecked() && audioSink) {
            audioSink->playMono16Sound(IAudioSink::Sound::Test);
        }
    }
}

void AVForm::on_cbEnableTestSound_stateChanged()
{
    audioSettings->setEnableTestSound(cbEnableTestSound->isChecked());

    if (cbEnableTestSound->isChecked() && audio.isOutputReady() && audioSink) {
        audioSink->playMono16Sound(IAudioSink::Sound::Test);
    }
}

void AVForm::on_microphoneSlider_valueChanged(int sliderSteps)
{
    const qreal dB = getValueFromSteps(sliderSteps, audio.minInputGain(), audio.maxInputGain());
    audioSettings->setAudioInGainDecibel(dB);
    audio.setInputGain(dB);
}

void AVForm::on_audioThresholdSlider_valueChanged(int sliderSteps)
{
    const qreal normThreshold =
        getValueFromSteps(sliderSteps, audio.minInputThreshold(), audio.maxInputThreshold());
    audioSettings->setAudioThreshold(normThreshold);
    audio.setInputThreshold(normThreshold);
}
void AVForm::createVideoSurface()
{
    if (camVideoSurface != nullptr)
        return;

    camVideoSurface = new VideoSurface(QPixmap(), CamFrame);
    camVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    camVideoSurface->setMinimumSize(QSize(160, 120));
    camVideoSurface->setSource(&camera);
    gridLayout->addWidget(camVideoSurface, 0, 0, 1, 1);
}

void AVForm::killVideoSurface()
{
    if (camVideoSurface == nullptr)
        return;

    QLayoutItem* child;
    while ((child = gridLayout->takeAt(0)) != nullptr)
        delete child;

    camVideoSurface->close();
    delete camVideoSurface;
    camVideoSurface = nullptr;
}

void AVForm::retranslateUi()
{
    Ui::AVForm::retranslateUi(this);
}

int AVForm::getStepsFromValue(qreal val, qreal valMin, qreal valMax) const
{
    const float norm = (val - valMin) / (valMax - valMin);
    return norm * totalSliderSteps;
}

qreal AVForm::getValueFromSteps(int steps, qreal valMin, qreal valMax) const
{
    return (static_cast<qreal>(steps) / totalSliderSteps) * (valMax - valMin) + valMin;
}
