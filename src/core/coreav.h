/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/toxcall.h"

#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <atomic>
#include <memory>
#include <tox/toxav.h>

class Friend;
class Conference;
class IAudioControl;
class IAudioSettings;
class IConferenceSettings;
class QThread;
class QTimer;
class CoreVideoSource;
class CameraSource;
class VideoSource;
class VideoFrame;
class Core;
struct vpx_image;

class CoreAV : public QObject
{
    Q_OBJECT

public:
    using CoreAVPtr = std::unique_ptr<CoreAV>;
    static CoreAVPtr makeCoreAV(Tox* core, QRecursiveMutex& toxCoreLock, IAudioSettings& audioSettings,
                                IConferenceSettings& conferenceSettings, CameraSource& cameraSource);

    void setAudio(IAudioControl& newAudio);
    IAudioControl* getAudio();

    ~CoreAV();

    bool isCallStarted(const Friend* f) const;
    bool isCallStarted(const Conference* c) const;
    bool isCallActive(const Friend* f) const;
    bool isCallActive(const Conference* c) const;
    bool isCallVideoEnabled(const Friend* f) const;
    bool sendCallAudio(uint32_t callId, const int16_t* pcm, size_t samples, uint8_t chans,
                       uint32_t rate) const;
    void sendCallVideo(uint32_t callId, std::shared_ptr<VideoFrame> frame);
    bool sendConferenceCallAudio(int conferenceNum, const int16_t* pcm, size_t samples,
                                 uint8_t chans, uint32_t rate) const;

    VideoSource* getVideoSourceFromCall(int friendNum) const;
    void sendNoVideo();

    void joinConferenceCall(const Conference& conference);
    void leaveConferenceCall(int conferenceNum);
    void muteCallInput(const Conference* c, bool mute);
    void muteCallOutput(const Conference* c, bool mute);
    bool isConferenceCallInputMuted(const Conference* c) const;
    bool isConferenceCallOutputMuted(const Conference* c) const;

    bool isCallInputMuted(const Friend* f) const;
    bool isCallOutputMuted(const Friend* f) const;
    void toggleMuteCallInput(const Friend* f);
    void toggleMuteCallOutput(const Friend* f);
    static void conferenceCallCallback(void* tox, uint32_t conference, uint32_t peer,
                                       const int16_t* data, unsigned samples, uint8_t channels,
                                       uint32_t sample_rate, void* core);
    void invalidateConferenceCallPeerSource(const Conference& conference, ToxPk peerPk);
    bool isAnyCallActive() const;

public slots:
    bool startCall(uint32_t friendNum, bool video);
    bool answerCall(uint32_t friendNum, bool video);
    bool cancelCall(uint32_t friendNum);
    void timeoutCall(uint32_t friendNum);
    void start();

signals:
    void avInvite(uint32_t friendId, bool video);
    void avStart(uint32_t friendId, bool video);
    void avEnd(uint32_t friendId, bool error = false);

private slots:
    static void callCallback(ToxAV* toxAV, uint32_t friendNum, bool audio, bool video, void* self);
    static void stateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t state, void* self);
    static void bitrateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t audioRate,
                                uint32_t videoRate, void* self);
    static void audioBitrateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t rate, void* self);
    static void videoBitrateCallback(ToxAV* toxAV, uint32_t friendNum, uint32_t rate, void* self);

private:
    struct ToxAVDeleter
    {
        void operator()(ToxAV* tox)
        {
            toxav_kill(tox);
        }
    };

    CoreAV(std::unique_ptr<ToxAV, ToxAVDeleter> toxav_, QRecursiveMutex& toxCoreLock,
           IAudioSettings& audioSettings_, IConferenceSettings& conferenceSettings_,
           CameraSource& cameraSource);
    void connectCallbacks();

    void process();
    static void audioFrameCallback(ToxAV* toxAV, uint32_t friendNum, const int16_t* pcm,
                                   size_t sampleCount, uint8_t channels, uint32_t samplingRate,
                                   void* self);
    static void videoFrameCallback(ToxAV* toxAV, uint32_t friendNum, uint16_t w, uint16_t h,
                                   const uint8_t* y, const uint8_t* u, const uint8_t* v,
                                   int32_t yStride, int32_t uStride, int32_t vStride, void* self);

private:
    static constexpr uint32_t VIDEO_DEFAULT_BITRATE = 2500;

private:
    // atomic because potentially accessed by different threads
    std::atomic<IAudioControl*> audio;
    std::unique_ptr<ToxAV, ToxAVDeleter> toxav;
    std::unique_ptr<QThread> coreAvThread;
    QTimer* iterateTimer = nullptr;
    using ToxFriendCallPtr = std::unique_ptr<ToxFriendCall>;
    /**
     * @brief Maps friend IDs to ToxFriendCall.
     * @note Need to use STL container here, because Qt containers need a copy constructor.
     */
    std::map<uint32_t, ToxFriendCallPtr> calls;


    using ToxConferenceCallPtr = std::unique_ptr<ToxConferenceCall>;
    /**
     * @brief Maps conference IDs to ToxConferenceCalls.
     * @note Need to use STL container here, because Qt containers need a copy constructor.
     */
    std::map<int, ToxConferenceCallPtr> conferenceCalls;

    // protect 'calls' and 'conferenceCalls'
    mutable QReadWriteLock callsLock{QReadWriteLock::Recursive};

    /**
     * @brief needed to synchronize with the Core thread, some toxav_* functions
     *        must not execute at the same time as tox_iterate()
     * @note This must be a recursive mutex as we're going to lock it in callbacks
     */
    QRecursiveMutex& coreLock;

    IAudioSettings& audioSettings;
    IConferenceSettings& conferenceSettings;
    CameraSource& cameraSource;
};
