/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include "genericsettings.h"
#include "ui_avform.h"
#include "src/video/videomode.h"

#include <memory>

class IAudioControl;
class IAudioSettings;
class IAudioSink;
class IAudioSource;
class CameraSource;
class CoreAV;
class IVideoSettings;
class VideoSurface;
class Style;
class AVForm : public GenericForm, private Ui::AVForm
{
    Q_OBJECT
public:
    AVForm(IAudioControl& audio_, CoreAV* coreAV_, CameraSource& camera_,
           IAudioSettings* audioSettings_, IVideoSettings* videoSettings_, Style& style);
    ~AVForm() override;
    QString getFormName() final
    {
        return tr("Audio/Video");
    }

private:
    void getAudioInDevices();
    void getAudioOutDevices();
    void getVideoDevices();

    static int getModeSize(VideoMode mode);
    void selectBestModes(QVector<VideoMode>& allVideoModes);
    void fillCameraModesComboBox();
    void fillScreenModesComboBox();
    void fillAudioQualityComboBox();
    int searchPreferredIndex();

    void createVideoSurface();
    void killVideoSurface();

    void retranslateUi();

private slots:
    // audio
    void on_inDevCombobox_currentIndexChanged(int deviceIndex);
    void on_outDevCombobox_currentIndexChanged(int deviceIndex);
    void on_playbackSlider_valueChanged(int sliderSteps);
    void on_cbEnableTestSound_stateChanged();
    void on_microphoneSlider_valueChanged(int sliderSteps);
    void on_audioThresholdSlider_valueChanged(int sliderSteps);
    void on_audioQualityComboBox_currentIndexChanged(int index);

    // camera
    void on_videoDevCombobox_currentIndexChanged(int index);
    void on_videoModescomboBox_currentIndexChanged(int index);

    void rescanDevices();
    void setVolume(qreal value);

protected:
    void updateVideoModes(int curIndex);

private:
    void hideEvent(QHideEvent* event) final;
    void showEvent(QShowEvent* event) final;
    void open(const QString& devName, const VideoMode& mode);
    int getStepsFromValue(qreal val, qreal valMin, qreal valMax);
    qreal getValueFromSteps(int steps, qreal valMin, qreal valMax);
    void trackNewScreenGeometry(QScreen* qScreen);

private:
    IAudioControl& audio;
    CoreAV* coreAV;
    IAudioSettings* audioSettings;
    IVideoSettings* videoSettings;

    bool subscribedToAudioIn;
    std::unique_ptr<IAudioSink> audioSink;
    std::unique_ptr<IAudioSource> audioSrc;
    VideoSurface* camVideoSurface;
    CameraSource& camera;
    QVector<QPair<QString, QString>> videoDeviceList;
    QVector<VideoMode> videoModes;
    uint alSource;
    const uint totalSliderSteps = 100; // arbitrary number of steps to give slider a good "feel"
};
