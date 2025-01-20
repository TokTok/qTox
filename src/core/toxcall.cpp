/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/core/toxcall.h"
#include "audio/audio.h"
#include "src/core/coreav.h"
#include "src/model/conference.h"
#include "src/persistence/settings.h"
#include "src/video/camerasource.h"
#include "src/video/corevideosource.h"
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

/**
 * @var uint32_t ToxCall::callId
 * @brief Could be a friendNum or conferenceNum, must uniquely identify the call. Do not modify!
 *
 * @var bool ToxCall::inactive
 * @brief True while we're not participating. (stopped conference call, ringing but hasn't started yet,
 * ...)
 *
 * @var bool ToxFriendCall::videoEnabled
 * @brief True if our user asked for a video call, sending and receiving.
 *
 * @var bool ToxFriendCall::nullVideoBitrate
 * @brief True if our video bitrate is zero, i.e. if the device is closed.
 *
 * @var TOXAV_FRIEND_CALL_STATE ToxFriendCall::state
 * @brief State of the peer (not ours!)
 *
 * @var QMap ToxConferenceCall::peers
 * @brief Keeps sources for users in conference calls.
 */

ToxCall::ToxCall(bool VideoEnabled_, CoreAV& av_, IAudioControl& audio_)
    : av{&av_}
    , audio(audio_)
    , videoEnabled{VideoEnabled_}
    , audioSource(audio_.makeSource())
{
}

ToxCall::~ToxCall()
{
    if (videoEnabled) {
        QObject::disconnect(videoInConn);
    }
}

bool ToxCall::isActive() const
{
    return active;
}

void ToxCall::setActive(bool value)
{
    active = value;
}

bool ToxCall::getMuteVol() const
{
    return muteVol;
}

void ToxCall::setMuteVol(bool value)
{
    muteVol = value;
}

bool ToxCall::getMuteMic() const
{
    return muteMic;
}

void ToxCall::setMuteMic(bool value)
{
    muteMic = value;
}

bool ToxCall::getVideoEnabled() const
{
    return videoEnabled;
}

void ToxCall::setVideoEnabled(bool value)
{
    videoEnabled = value;
}

bool ToxCall::getNullVideoBitrate() const
{
    return nullVideoBitrate;
}

void ToxCall::setNullVideoBitrate(bool value)
{
    nullVideoBitrate = value;
}

CoreVideoSource* ToxCall::getVideoSource() const
{
    return videoSource;
}

ToxFriendCall::ToxFriendCall(uint32_t friendNum, bool VideoEnabled, CoreAV& av_,
                             IAudioControl& audio_, CameraSource& cameraSource_)
    : ToxCall(VideoEnabled, av_, audio_)
    , sink(audio_.makeSink())
    , friendId{friendNum}
    , cameraSource{cameraSource_}
{
    connect(audioSource.get(), &IAudioSource::frameAvailable, &av_,
            [&av_, friendNum](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                av_.sendCallAudio(friendNum, pcm, samples, chans, rate);
            });

    audioSourceInvalid = connect(audioSource.get(), &IAudioSource::invalidated, this,
                                 &ToxFriendCall::onAudioSourceInvalidated);

    if (sink) {
        audioSinkInvalid = sink->connectTo_invalidated(this, [this]() { onAudioSinkInvalidated(); });
    }

    // register video
    if (videoEnabled) {
        videoSource = new CoreVideoSource();

        if (cameraSource.isNone()) {
            cameraSource.setupDefault();
        }
        cameraSource.subscribe();
        videoInConn = QObject::connect(&cameraSource, &VideoSource::frameAvailable, &av_,
                                       [&av_, friendNum](std::shared_ptr<VideoFrame> frame) {
                                           av_.sendCallVideo(friendNum, frame);
                                       });
        if (videoInConn == nullptr) {
            qDebug() << "Video connection not working";
        }
    }
}

ToxFriendCall::~ToxFriendCall()
{
    if (videoEnabled) {
        cameraSource.unsubscribe();
    }
    QObject::disconnect(audioSinkInvalid);
    QObject::disconnect(audioSourceInvalid);
}

void ToxFriendCall::onAudioSourceInvalidated()
{
    auto newSrc = audio.makeSource();
    if (newSrc == nullptr) {
        qWarning() << "Failed to create a new audio source";
        return;
    }
    connect(newSrc.get(), &IAudioSource::frameAvailable, this,
            [this](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                av->sendCallAudio(friendId, pcm, samples, chans, rate);
            });
    audioSource = std::move(newSrc);

    connect(audioSource.get(), &IAudioSource::invalidated, this,
            &ToxFriendCall::onAudioSourceInvalidated);
}

void ToxFriendCall::onAudioSinkInvalidated()
{
    auto newSink = audio.makeSink();

    if (newSink) {
        audioSinkInvalid =
            newSink->connectTo_invalidated(this, [this]() { onAudioSinkInvalidated(); });
    }

    sink = std::move(newSink);
}

TOXAV_FRIEND_CALL_STATE ToxFriendCall::getState() const
{
    return state;
}

void ToxFriendCall::setState(const TOXAV_FRIEND_CALL_STATE& value)
{
    state = value;
}

void ToxFriendCall::playAudioBuffer(const int16_t* data, int samples, unsigned channels,
                                    int sampleRate) const
{
    if (sink) {
        sink->playAudioBuffer(data, samples, channels, sampleRate);
    }
}

ToxConferenceCall::ToxConferenceCall(const Conference& conference_, CoreAV& av_, IAudioControl& audio_)
    : ToxCall(false, av_, audio_)
    , conference{conference_}
{
    if (audioSource == nullptr) {
        qWarning() << "Created conference call without working audio source";
        return;
    }

    // register audio
    connect(audioSource.get(), &IAudioSource::frameAvailable, this,
            [this](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                if (conference.getPeersCount() <= 1) {
                    return;
                }

                av->sendConferenceCallAudio(conference.getId(), pcm, samples, chans, rate);
            });

    connect(audioSource.get(), &IAudioSource::invalidated, this,
            &ToxConferenceCall::onAudioSourceInvalidated);
}

ToxConferenceCall::~ToxConferenceCall()
{
    // disconnect all Qt connections
    clearPeers();
}

void ToxConferenceCall::onAudioSourceInvalidated()
{
    auto newSrc = audio.makeSource();
    if (newSrc == nullptr) {
        qWarning() << "Failed to create a new audio source";
        return;
    }
    connect(newSrc.get(), &IAudioSource::frameAvailable, this,
            [this](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                if (conference.getPeersCount() <= 1) {
                    return;
                }

                av->sendConferenceCallAudio(conference.getId(), pcm, samples, chans, rate);
            });

    audioSource = std::move(newSrc);

    connect(audioSource.get(), &IAudioSource::invalidated, this,
            &ToxConferenceCall::onAudioSourceInvalidated);
}


void ToxConferenceCall::onAudioSinkInvalidated(ToxPk peerId)
{
    removePeer(peerId);
    addPeer(peerId);
}

void ToxConferenceCall::removePeer(ToxPk peerId)
{
    const auto& source = peers.find(peerId);
    if (source == peers.cend()) {
        qDebug() << "Peer:" << peerId.toString() << "does not have a source, can't remove";
        return;
    }

    peers.erase(source);
    QObject::disconnect(sinkInvalid[peerId]);
    sinkInvalid.erase(peerId);
}

void ToxConferenceCall::addPeer(ToxPk peerId)
{
    std::unique_ptr<IAudioSink> newSink = audio.makeSink();

    QMetaObject::Connection con;

    if (newSink) {
        con = newSink->connectTo_invalidated(this,
                                             [this, peerId]() { onAudioSinkInvalidated(peerId); });
    }

    peers.emplace(peerId, std::move(newSink));
    sinkInvalid.insert({peerId, con});
}

bool ToxConferenceCall::havePeer(ToxPk peerId)
{
    const auto& source = peers.find(peerId);
    return source != peers.cend();
}

void ToxConferenceCall::clearPeers()
{
    peers.clear();
    for (auto con : sinkInvalid) {
        QObject::disconnect(con.second);
    }

    sinkInvalid.clear();
}

void ToxConferenceCall::playAudioBuffer(const ToxPk& peer, const int16_t* data, int samples,
                                        unsigned channels, int sampleRate)
{
    if (!havePeer(peer)) {
        addPeer(peer);
    }
    const auto& source = peers.find(peer);
    if (source->second) {
        source->second->playAudioBuffer(data, samples, channels, sampleRate);
    }
}
