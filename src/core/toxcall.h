/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "audio/iaudiocontrol.h"
#include "audio/iaudiosink.h"
#include "audio/iaudiosource.h"
#include <src/core/toxpk.h>
#include <tox/toxav.h>

#include <QMap>
#include <QMetaObject>
#include <QtGlobal>

#include <cstdint>
#include <memory>

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;
class Conference;
class CameraSource;

class ToxCall : public QObject
{
    Q_OBJECT

protected:
    ToxCall() = delete;
    ToxCall(bool VideoEnabled, CoreAV& av, IAudioControl& audio);
    ~ToxCall();

public:
    ToxCall(const ToxCall& other) = delete;
    ToxCall(ToxCall&& other) = delete;

    ToxCall& operator=(const ToxCall& other) = delete;
    ToxCall& operator=(ToxCall&& other) = delete;

    bool isActive() const;
    void setActive(bool value);

    bool getMuteVol() const;
    void setMuteVol(bool value);

    bool getMuteMic() const;
    void setMuteMic(bool value);

    bool getVideoEnabled() const;
    void setVideoEnabled(bool value);

    bool getNullVideoBitrate() const;
    void setNullVideoBitrate(bool value);

    CoreVideoSource* getVideoSource() const;

protected:
    bool active{false};
    CoreAV* av{nullptr};
    // audio
    IAudioControl& audio;
    bool muteMic{false};
    bool muteVol{false};
    // video
    CoreVideoSource* videoSource{nullptr};
    QMetaObject::Connection videoInConn;
    bool videoEnabled{false};
    bool nullVideoBitrate{false};
    std::unique_ptr<IAudioSource> audioSource;
};

class ToxFriendCall : public ToxCall
{
    Q_OBJECT
public:
    ToxFriendCall() = delete;
    ToxFriendCall(uint32_t friendNum, bool VideoEnabled, CoreAV& av_, IAudioControl& audio_,
                  CameraSource& cameraSource);
    ToxFriendCall(ToxFriendCall&& other) = delete;
    ToxFriendCall& operator=(ToxFriendCall&& other) = delete;
    ~ToxFriendCall();

    TOXAV_FRIEND_CALL_STATE getState() const;
    void setState(const TOXAV_FRIEND_CALL_STATE& value);

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const;

private slots:
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated();

private:
    QMetaObject::Connection audioSinkInvalid;
    TOXAV_FRIEND_CALL_STATE state{TOXAV_FRIEND_CALL_STATE_NONE};
    std::unique_ptr<IAudioSink> sink;
    uint32_t friendId;
    CameraSource& cameraSource;
};

class ToxConferenceCall : public ToxCall
{
    Q_OBJECT
public:
    ToxConferenceCall() = delete;
    ToxConferenceCall(const Conference& conference_, CoreAV& av_, IAudioControl& audio_);
    ToxConferenceCall(ToxConferenceCall&& other) = delete;
    ~ToxConferenceCall();

    ToxConferenceCall& operator=(ToxConferenceCall&& other) = delete;
    void removePeer(ToxPk peerId);

    void playAudioBuffer(const ToxPk& peer, const int16_t* data, int samples, unsigned channels,
                         int sampleRate);

private:
    void addPeer(ToxPk peerId);
    bool havePeer(ToxPk peerId);
    void clearPeers();

    std::map<ToxPk, std::unique_ptr<IAudioSink>> peers;
    std::map<ToxPk, QMetaObject::Connection> sinkInvalid;
    const Conference& conference;

private slots:
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated(ToxPk peerId);
};
