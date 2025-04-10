/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2013 by Maxim Biro <nurupo.contributions@gmail.com>
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "coreav.h"

#include "core.h"

#include "audio/iaudiosettings.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/persistence/iconferencesettings.h"
#include "src/video/corevideosource.h"
#include "src/video/videoframe.h"
#include "util/algorithm.h"
#include "util/toxcoreerrorparser.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>

#include <cassert>
#include <memory>
#include <tox/toxav.h>

/**
 * @fn void CoreAV::avInvite(uint32_t friendId, bool video)
 * @brief Sent when a friend calls us.
 * @param friendId Id of friend in call list.
 * @param video False if chat is audio only, true audio and video.
 *
 * @fn void CoreAV::avStart(uint32_t friendId, bool video)
 * @brief Sent when a call we initiated has started.
 * @param friendId Id of friend in call list.
 * @param video False if chat is audio only, true audio and video.
 *
 * @fn void CoreAV::avEnd(uint32_t friendId)
 * @brief Sent when a call was ended by the peer.
 * @param friendId Id of friend in call list.
 *
 * @var CoreAV::VIDEO_DEFAULT_BITRATE
 * @brief Picked at random by fair dice roll.
 */

/**
 * @var std::atomic_flag CoreAV::threadSwitchLock
 * @brief This flag is to be acquired before switching in a blocking way between the UI and CoreAV
 * thread.
 *
 * The CoreAV thread must have priority for the flag, other threads should back off or release it
 * quickly.
 * CoreAV needs to interface with three threads, the toxcore/Core thread that fires non-payload
 * toxav callbacks, the toxav/CoreAV thread that fires AV payload callbacks and manages
 * most of CoreAV's members, and the UI thread, which calls our [start/answer/cancel]Call functions
 * and which we call via signals.
 * When the UI calls us, we switch from the UI thread to the CoreAV thread to do the processing,
 * when toxcore fires a non-payload av callback, we do the processing in the CoreAV thread and then
 * switch to the UI thread to send it a signal. Both switches block both threads, so this would
 * deadlock.
 */

CoreAV::CoreAV(std::unique_ptr<ToxAV, ToxAVDeleter> toxav_, QRecursiveMutex& toxCoreLock,
               IAudioSettings& audioSettings_, IConferenceSettings& conferenceSettings_,
               CameraSource& cameraSource_)
    : audio{nullptr}
    , isCancelling(false)
    , toxav{std::move(toxav_)}
    , coreAvThread{new QThread{this}}
    , iterateTimer{new QTimer{this}}
    , coreLock{toxCoreLock}
    , audioSettings{audioSettings_}
    , conferenceSettings{conferenceSettings_}
    , cameraSource{cameraSource_}
{
    assert(coreAvThread);
    assert(iterateTimer);

    coreAvThread->setObjectName("qTox CoreAV");
    moveToThread(coreAvThread.get());

    connectCallbacks();

    iterateTimer->setSingleShot(true);

    connect(iterateTimer, &QTimer::timeout, this, &CoreAV::process);
    connect(coreAvThread.get(), &QThread::finished, iterateTimer, &QTimer::stop);
    connect(coreAvThread.get(), &QThread::started, this, &CoreAV::process);
}

void CoreAV::connectCallbacks()
{
    toxav_callback_call(toxav.get(), CoreAV::callCallback, this);
    toxav_callback_call_state(toxav.get(), CoreAV::stateCallback, this);
    toxav_callback_audio_bit_rate(toxav.get(), CoreAV::audioBitrateCallback, this);
    toxav_callback_video_bit_rate(toxav.get(), CoreAV::videoBitrateCallback, this);
    toxav_callback_audio_receive_frame(toxav.get(), CoreAV::audioFrameCallback, this);
    toxav_callback_video_receive_frame(toxav.get(), CoreAV::videoFrameCallback, this);
}

/**
 * @brief Factory method for CoreAV
 * @param core pointer to the Tox instance
 * @return CoreAV instance on success, {} on failure
 */
CoreAV::CoreAVPtr CoreAV::makeCoreAV(Tox* core, QRecursiveMutex& toxCoreLock,
                                     IAudioSettings& audioSettings,
                                     IConferenceSettings& conferenceSettings,
                                     CameraSource& cameraSource)
{
    Toxav_Err_New err;
    std::unique_ptr<ToxAV, ToxAVDeleter> toxav{toxav_new(core, &err)};
    switch (err) {
    case TOXAV_ERR_NEW_OK:
        break;
    case TOXAV_ERR_NEW_MALLOC:
        qCritical() << "Failed to allocate resources for ToxAV";
        return {};
    case TOXAV_ERR_NEW_MULTIPLE:
        qCritical() << "Attempted to create multiple ToxAV instances";
        return {};
    case TOXAV_ERR_NEW_NULL:
        qCritical() << "Unexpected NULL parameter";
        return {};
    }

    assert(toxav != nullptr);

    return CoreAVPtr{
        new CoreAV{std::move(toxav), toxCoreLock, audioSettings, conferenceSettings, cameraSource}};
}

/**
 * @brief Set the audio backend
 * @param audio The audio backend to use
 * @note This must be called before starting CoreAV and audio must outlive CoreAV
 */
void CoreAV::setAudio(IAudioControl& newAudio)
{
    audio.exchange(&newAudio);
}

/**
 * @brief Get the audio backend used
 * @return Pointer to the audio backend
 * @note This is needed only for the case CoreAV needs to restart and the restarting class doesn't
 * have access to the audio backend and wants to keep it the same.
 */
IAudioControl* CoreAV::getAudio()
{
    return audio;
}

CoreAV::~CoreAV()
{
    /* Gracefully leave calls and conference calls to avoid deadlocks in destructor */
    forEachKey(calls, [this](uint32_t callId) { cancelCall(callId); });
    forEachKey(conferenceCalls, [this](int callId) { leaveConferenceCall(callId); });

    assert(calls.empty());
    assert(conferenceCalls.empty());

    coreAvThread->exit(0);
    coreAvThread->wait();
}

/**
 * @brief Starts the CoreAV main loop that calls toxav's main loop
 */
void CoreAV::start()
{
    coreAvThread->start();
}

void CoreAV::process()
{
    assert(QThread::currentThread() == coreAvThread.get());
    toxav_iterate(toxav.get());
    iterateTimer->start(toxav_iteration_interval(toxav.get()));
}

/**
 * @brief Checks the call status for a Tox friend.
 * @param f the friend to check
 * @return true, if call is started for the friend, false otherwise
 */
bool CoreAV::isCallStarted(const Friend* f) const
{
    const QReadLocker locker{&callsLock};
    return (f != nullptr) && calls.contains(f->getId());
}

/**
 * @brief Checks the call status for a Tox conference.
 * @param c the conference to check
 * @return true, if call is started for the conference, false otherwise
 */
bool CoreAV::isCallStarted(const Conference* c) const
{
    const QReadLocker locker{&callsLock};
    return (c != nullptr) && conferenceCalls.contains(c->getId());
}

/**
 * @brief Checks the call status for a Tox friend.
 * @param f the friend to check
 * @return true, if call is active for the friend, false otherwise
 */
bool CoreAV::isCallActive(const Friend* f) const
{
    const QReadLocker locker{&callsLock};
    auto it = calls.find(f->getId());
    if (it == calls.end()) {
        return false;
    }
    return isCallStarted(f) && it->second->isActive();
}

/**
 * @brief Checks the call status for a Tox conference.
 * @param c the conference to check
 * @return true, if the call is active for the conference, false otherwise
 */
bool CoreAV::isCallActive(const Conference* c) const
{
    const QReadLocker locker{&callsLock};
    auto it = conferenceCalls.find(c->getId());
    if (it == conferenceCalls.end()) {
        return false;
    }
    return isCallStarted(c) && it->second->isActive();
}

bool CoreAV::isCallVideoEnabled(const Friend* f) const
{
    const QReadLocker locker{&callsLock};
    auto it = calls.find(f->getId());
    return isCallStarted(f) && it->second->getVideoEnabled();
}

bool CoreAV::answerCall(uint32_t friendNum, bool video)
{
    const QWriteLocker locker{&callsLock};
    const QMutexLocker<QRecursiveMutex> coreLocker{&coreLock};

    qDebug() << "Answering call" << friendNum;
    auto it = calls.find(friendNum);
    assert(it != calls.end());
    Toxav_Err_Answer err;

    const uint32_t videoBitrate = video ? VIDEO_DEFAULT_BITRATE : 0;
    if (toxav_answer(toxav.get(), friendNum, audioSettings.getAudioBitrate(), videoBitrate, &err)) {
        it->second->setActive(true);
        return true;
    }
    qWarning() << "Failed to answer call with error" << err;
    Toxav_Err_Call_Control controlErr;
    toxav_call_control(toxav.get(), friendNum, TOXAV_CALL_CONTROL_CANCEL, &controlErr);
    PARSE_ERR(controlErr);
    calls.erase(it);
    return false;
}

bool CoreAV::startCall(uint32_t friendNum, bool video)
{
    const QWriteLocker locker{&callsLock};
    const QMutexLocker<QRecursiveMutex> coreLocker{&coreLock};

    qDebug() << "Starting call with" << friendNum;
    auto it = calls.find(friendNum);
    if (it != calls.end()) {
        qWarning() << QString("Can't start call with %1, we're already in this call!").arg(friendNum);
        return false;
    }

    const uint32_t videoBitrate = video ? VIDEO_DEFAULT_BITRATE : 0;
    Toxav_Err_Call err;
    toxav_call(toxav.get(), friendNum, audioSettings.getAudioBitrate(), videoBitrate, &err);
    if (!PARSE_ERR(err)) {
        return false;
    }

    // Audio backend must be set before making a call
    assert(audio != nullptr);
    ToxFriendCallPtr call =
        std::make_unique<ToxFriendCall>(friendNum, video, *this, *audio, cameraSource);
    // Call object must be owned by this thread or there will be locking problems with Audio
    call->moveToThread(thread());
    assert(call != nullptr);
    calls.emplace(friendNum, std::move(call));
    return true;
}

bool CoreAV::cancelCall(uint32_t friendNum)
{
    isCancelling = true;
    QWriteLocker locker{&callsLock};
    const QMutexLocker<QRecursiveMutex> coreLocker{&coreLock};

    qDebug() << "Cancelling call with" << friendNum;
    Toxav_Err_Call_Control err;
    toxav_call_control(toxav.get(), friendNum, TOXAV_CALL_CONTROL_CANCEL, &err);
    if (!PARSE_ERR(err)) {
        return false;
    }

    calls.erase(friendNum);
    locker.unlock();

    emit avEnd(friendNum);
    isCancelling = false;
    return true;
}

void CoreAV::timeoutCall(uint32_t friendNum)
{
    const QWriteLocker locker{&callsLock};

    if (!cancelCall(friendNum)) {
        qWarning() << QString("Failed to timeout call with %1").arg(friendNum);
        return;
    }
    qDebug() << "Call with friend" << friendNum << "timed out";
}

/**
 * @brief Send audio frame to a friend
 * @param callId Id of friend in call list.
 * @param pcm An array of audio samples (Pulse-code modulation).
 * @param samples Number of samples in this frame.
 * @param chans Number of audio channels.
 * @param rate Audio sampling rate used in this frame.
 * @return False only on error, but not if there's nothing to send.
 */
bool CoreAV::sendCallAudio(uint32_t callId, const int16_t* pcm, size_t samples, uint8_t chans,
                           uint32_t rate) const
{
    const QReadLocker locker{&callsLock};

    auto it = calls.find(callId);
    if (it == calls.end()) {
        return false;
    }

    const ToxFriendCall& call = *it->second;

    if (call.getMuteMic() || !call.isActive()
        || ((call.getState() & TOXAV_FRIEND_CALL_STATE_ACCEPTING_A) == 0)) {
        return true;
    }

    // TOXAV_ERR_SEND_FRAME_SYNC means toxav failed to lock, retry 5 times in this case
    Toxav_Err_Send_Frame err;
    int retries = 0;
    do {
        if (!toxav_audio_send_frame(toxav.get(), callId, pcm, samples, chans, rate, &err)) {
            if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
                ++retries;
                QThread::usleep(500);
            } else {
                qDebug() << "toxav_audio_send_frame error:" << err;
            }
        }
    } while (err == TOXAV_ERR_SEND_FRAME_SYNC && retries < 5);
    if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
        qDebug() << "toxav_audio_send_frame error: Lock busy, dropping frame";
    }

    return true;
}

void CoreAV::sendCallVideo(uint32_t callId, std::shared_ptr<VideoFrame> vframe)
{
    const QWriteLocker locker{&callsLock};

    // We might be running in the FFmpeg thread and holding the CameraSource lock
    // So be careful not to deadlock with anything while toxav locks in toxav_video_send_frame
    auto it = calls.find(callId);
    if (it == calls.end()) {
        return;
    }

    ToxFriendCall& call = *it->second;

    if (!call.getVideoEnabled() || !call.isActive()
        || ((call.getState() & TOXAV_FRIEND_CALL_STATE_ACCEPTING_V) == 0)) {
        return;
    }

    if (call.getNullVideoBitrate()) {
        qDebug() << "Restarting video stream to friend" << callId;
        const QMutexLocker<QRecursiveMutex> coreLocker{&coreLock};
        Toxav_Err_Bit_Rate_Set err;
        toxav_video_set_bit_rate(toxav.get(), callId, VIDEO_DEFAULT_BITRATE, &err);
        if (!PARSE_ERR(err)) {
            return;
        }
        call.setNullVideoBitrate(false);
    }

    auto [frame, frameLocker] = vframe->toToxYUVFrame();

    if (!frame.isValid()) {
        return;
    }

    // TOXAV_ERR_SEND_FRAME_SYNC means toxav failed to lock, retry 5 times in this case
    // We don't want to be dropping iframes because of some lock held by toxav_iterate
    Toxav_Err_Send_Frame err;
    int retries = 0;
    do {
        if (!toxav_video_send_frame(toxav.get(), callId, frame.width, frame.height, frame.y,
                                    frame.u, frame.v, &err)) {
            if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
                ++retries;
                QThread::usleep(500);
            } else {
                qDebug() << "toxav_video_send_frame error:" << err;
            }
        }
    } while (err == TOXAV_ERR_SEND_FRAME_SYNC && retries < 5);
    if (err == TOXAV_ERR_SEND_FRAME_SYNC) {
        qDebug() << "toxav_video_send_frame error: Lock busy, dropping frame";
    }
}

/**
 * @brief Toggles the mute state of the call's input (microphone).
 * @param f The friend assigned to the call
 */
void CoreAV::toggleMuteCallInput(const Friend* f)
{
    const QWriteLocker locker{&callsLock};

    auto it = calls.find(f->getId());
    if ((f != nullptr) && (it != calls.end())) {
        ToxCall& call = *it->second;
        call.setMuteMic(!call.getMuteMic());
    }
}

/**
 * @brief Toggles the mute state of the call's output (speaker).
 * @param f The friend assigned to the call
 */
void CoreAV::toggleMuteCallOutput(const Friend* f)
{
    const QWriteLocker locker{&callsLock};

    auto it = calls.find(f->getId());
    if ((f != nullptr) && (it != calls.end())) {
        ToxCall& call = *it->second;
        call.setMuteVol(!call.getMuteVol());
    }
}

/**
 * @brief Called from Tox API when conference call receives audio data.
 *
 * @param[in] tox          the Tox object
 * @param[in] conference   the conference number
 * @param[in] peer         the peer number
 * @param[in] data         the audio data to playback
 * @param[in] samples      the audio samples
 * @param[in] channels     the audio channels
 * @param[in] sample_rate  the audio sample rate
 * @param[in] core         the qTox Core class
 */
void CoreAV::conferenceCallCallback(void* tox, uint32_t conference, uint32_t peer,
                                    const int16_t* data, unsigned samples, uint8_t channels,
                                    uint32_t sample_rate, void* core)
{
    /*
     * Currently conference call audio decoding is handled in the Tox thread by c-toxcore,
     * so we can be sure that this function is always called from the Core thread.
     * To change this, an API change in c-toxcore is needed and this function probably must be
     * changed.
     * See https://github.com/TokTok/c-toxcore/issues/1364 for details.
     */

    std::ignore = tox;
    Core* c = static_cast<Core*>(core);
    CoreAV* cav = c->getAv();

    const QReadLocker locker{&cav->callsLock};

    const ToxPk peerPk = c->getConferencePeerPk(conference, peer);
    // don't play the audio if it comes from a muted peer
    if (cav->conferenceSettings.getBlockList().contains(peerPk.toString())) {
        return;
    }

    emit c->conferencePeerAudioPlaying(conference, peerPk);

    auto it = cav->conferenceCalls.find(conference);
    if (it == cav->conferenceCalls.end()) {
        return;
    }

    ToxConferenceCall& call = *it->second;

    if (call.getMuteVol() || !call.isActive()) {
        return;
    }

    call.playAudioBuffer(peerPk, data, samples, channels, sample_rate);
}

/**
 * @brief Called from core to make sure the source for that peer is invalidated when they leave.
 * @param conference Conference Index
 * @param peer Peer Index
 */
void CoreAV::invalidateConferenceCallPeerSource(const Conference& conference, ToxPk peerPk)
{
    const QWriteLocker locker{&callsLock};

    auto it = conferenceCalls.find(conference.getId());
    if (it == conferenceCalls.end()) {
        return;
    }
    it->second->removePeer(peerPk);
}

bool CoreAV::isAnyCallActive() const
{
    const QReadLocker locker{&callsLock};
    return std::any_of(calls.begin(), calls.end(),
                       [](const auto& call) { return call.second->isActive(); });
}

/**
 * @brief Get a call's video source.
 * @param friendNum Id of friend in call list.
 * @return Video surface to show
 */
VideoSource* CoreAV::getVideoSourceFromCall(int friendNum) const
{
    const QReadLocker locker{&callsLock};

    auto it = calls.find(friendNum);
    if (it == calls.end()) {
        qWarning() << "CoreAV::getVideoSourceFromCall: No such call, possibly cancelled";
        return nullptr;
    }

    return it->second->getVideoSource();
}

/**
 * @brief Starts a call in an existing AV conference.
 * @note Call from the GUI thread.
 * @param conferenceId Id of conference to join
 */
void CoreAV::joinConferenceCall(const Conference& conference)
{
    const QWriteLocker locker{&callsLock};

    qDebug() << "Joining conference call" << conference.getId();

    // Audio backend must be set before starting a call
    assert(audio != nullptr);

    ToxConferenceCallPtr conferencecall =
        std::make_unique<ToxConferenceCall>(conference, *this, *audio);
    // Call Objects must be owned by CoreAV or there will be locking problems with Audio
    conferencecall->moveToThread(thread());
    assert(conferencecall != nullptr);
    auto ret = conferenceCalls.emplace(conference.getId(), std::move(conferencecall));
    if (!ret.second) {
        qWarning() << "This conference call already exists, not joining!";
        return;
    }
    ret.first->second->setActive(true);
}

/**
 * @brief Will not leave the conference, just stop the call.
 * @note Call from the GUI thread.
 * @param conferenceId Id of conference to leave
 */
void CoreAV::leaveConferenceCall(int conferenceNum)
{
    const QWriteLocker locker{&callsLock};

    qDebug() << "Leaving conference call" << conferenceNum;

    conferenceCalls.erase(conferenceNum);
}

bool CoreAV::sendConferenceCallAudio(int conferenceNum, const int16_t* pcm, size_t samples,
                                     uint8_t chans, uint32_t rate) const
{
    const QReadLocker locker{&callsLock};

    const auto it = conferenceCalls.find(conferenceNum);
    if (it == conferenceCalls.end()) {
        return false;
    }

    if (!it->second->isActive() || it->second->getMuteMic()) {
        return true;
    }

    if (toxav_group_send_audio(toxav_get_tox(toxav.get()), conferenceNum, pcm, samples, chans, rate)
        != 0)
        qDebug() << "toxav_group_send_audio error";

    return true;
}

/**
 * @brief Mutes or unmutes the conference call's input (microphone).
 * @param c The conference
 * @param mute True to mute, false to unmute
 */
void CoreAV::muteCallInput(const Conference* c, bool mute)
{
    const QWriteLocker locker{&callsLock};

    auto it = conferenceCalls.find(c->getId());
    if ((c != nullptr) && (it != conferenceCalls.end())) {
        it->second->setMuteMic(mute);
    }
}

/**
 * @brief Mutes or unmutes the conference call's output (speaker).
 * @param c The conference
 * @param mute True to mute, false to unmute
 */
void CoreAV::muteCallOutput(const Conference* c, bool mute)
{
    const QWriteLocker locker{&callsLock};

    auto it = conferenceCalls.find(c->getId());
    if ((c != nullptr) && (it != conferenceCalls.end())) {
        it->second->setMuteVol(mute);
    }
}

/**
 * @brief Returns the conference calls input (microphone) state.
 * @param conferenceId The conference id to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isConferenceCallInputMuted(const Conference* c) const
{
    const QReadLocker locker{&callsLock};

    if (c == nullptr) {
        return false;
    }

    const uint32_t conferenceId = c->getId();
    auto it = conferenceCalls.find(conferenceId);
    return (it != conferenceCalls.end()) && it->second->getMuteMic();
}

/**
 * @brief Returns the conference calls output (speaker) state.
 * @param conferenceId The conference id to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isConferenceCallOutputMuted(const Conference* c) const
{
    const QReadLocker locker{&callsLock};

    if (c == nullptr) {
        return false;
    }

    const uint32_t conferenceId = c->getId();
    auto it = conferenceCalls.find(conferenceId);
    return (it != conferenceCalls.end()) && it->second->getMuteVol();
}

/**
 * @brief Returns the calls input (microphone) mute state.
 * @param f The friend to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isCallInputMuted(const Friend* f) const
{
    const QReadLocker locker{&callsLock};

    if (f == nullptr) {
        return false;
    }
    const uint32_t friendId = f->getId();
    auto it = calls.find(friendId);
    return (it != calls.end()) && it->second->getMuteMic();
}

/**
 * @brief Returns the calls output (speaker) mute state.
 * @param friendId The friend to check
 * @return true when muted, false otherwise
 */
bool CoreAV::isCallOutputMuted(const Friend* f) const
{
    const QReadLocker locker{&callsLock};

    if (f == nullptr) {
        return false;
    }
    const uint32_t friendId = f->getId();
    auto it = calls.find(friendId);
    return (it != calls.end()) && it->second->getMuteVol();
}

/**
 * @brief Signal to all peers that we're not sending video anymore.
 * @note The next frame sent cancels this.
 */
void CoreAV::sendNoVideo()
{
    const QWriteLocker locker{&callsLock};

    // We don't change the audio bitrate, but we signal that we're not sending video anymore
    qDebug() << "CoreAV: Signaling end of video sending";
    for (auto& kv : calls) {
        ToxFriendCall& call = *kv.second;
        Toxav_Err_Bit_Rate_Set err;
        toxav_video_set_bit_rate(toxav.get(), kv.first, 0, &err);
        if (!PARSE_ERR(err)) {
            continue;
        }
        call.setNullVideoBitrate(true);
    }
}

void CoreAV::callCallback(ToxAV* toxav, uint32_t friendNum, bool audio, bool video, void* vSelf)
{
    auto* self = static_cast<CoreAV*>(vSelf);

    QWriteLocker locker{&self->callsLock};

    // Audio backend must be set before receiving a call
    assert(self->audio != nullptr);
    ToxFriendCallPtr call =
        std::make_unique<ToxFriendCall>(friendNum, video, *self, *self->audio, self->cameraSource);
    // Call object must be owned by CoreAV thread or there will be locking problems with Audio
    call->moveToThread(self->thread());
    assert(call != nullptr);

    auto it = self->calls.emplace(friendNum, std::move(call));
    if (!it.second) {
        qWarning() << QString("Rejecting call invite from %1, we're already in that call!").arg(friendNum);
        Toxav_Err_Call_Control err;
        toxav_call_control(toxav, friendNum, TOXAV_CALL_CONTROL_CANCEL, &err);
        PARSE_ERR(err);
        return;
    }
    qDebug() << "Received call invite from" << friendNum;

    // We don't get a state callback when answering, so fill the state ourselves in advance
    int state = 0;
    if (audio)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_A | TOXAV_FRIEND_CALL_STATE_ACCEPTING_A;
    if (video)
        state |= TOXAV_FRIEND_CALL_STATE_SENDING_V | TOXAV_FRIEND_CALL_STATE_ACCEPTING_V;
    it.first->second->setState(static_cast<TOXAV_FRIEND_CALL_STATE>(state));

    // Must explicitly unlock, because a deadlock can happen via ChatForm/Audio
    locker.unlock();

    emit self->avInvite(friendNum, video);
}

void CoreAV::stateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t state, void* vSelf)
{
    std::ignore = toxav;
    auto* self = static_cast<CoreAV*>(vSelf);

    // we must unlock this lock before emitting any signals
    QWriteLocker locker{&self->callsLock};

    auto it = self->calls.find(friendNum);
    if (it == self->calls.end()) {
        qWarning() << QString("stateCallback called, but call %1 is already dead").arg(friendNum);
        return;
    }

    ToxFriendCall& call = *it->second;

    if ((state & TOXAV_FRIEND_CALL_STATE_ERROR) != 0u) {
        qWarning() << "Call with friend" << friendNum << "died of unnatural causes!";
        self->calls.erase(friendNum);
        locker.unlock();
        emit self->avEnd(friendNum, true);
    } else if ((state & TOXAV_FRIEND_CALL_STATE_FINISHED) != 0u) {
        qDebug() << "Call with friend" << friendNum << "finished quietly";
        self->calls.erase(friendNum);
        locker.unlock();
        emit self->avEnd(friendNum);
    } else {
        // If our state was null, we started the call and were still ringing
        if ((call.getState() == 0u) && (state != 0u)) {
            call.setActive(true);
            const bool videoEnabled = call.getVideoEnabled();
            call.setState(static_cast<TOXAV_FRIEND_CALL_STATE>(state));
            locker.unlock();
            emit self->avStart(friendNum, videoEnabled);
        } else if (((call.getState() & TOXAV_FRIEND_CALL_STATE_SENDING_V) != 0)
                   && ((state & TOXAV_FRIEND_CALL_STATE_SENDING_V) == 0u)) {
            qDebug() << "Friend" << friendNum << "stopped sending video";
            if (call.getVideoSource() != nullptr) {
                call.getVideoSource()->stopSource();
            }

            call.setState(static_cast<TOXAV_FRIEND_CALL_STATE>(state));
        } else if (((call.getState() & TOXAV_FRIEND_CALL_STATE_SENDING_V) == 0)
                   && ((state & TOXAV_FRIEND_CALL_STATE_SENDING_V) != 0u)) {
            // Workaround toxav sometimes firing callbacks for "send last frame" -> "stop sending
            // video"
            // out of orders (even though they were sent in order by the other end).
            // We simply stop the videoSource from emitting anything while the other end says it's
            // not sending
            if (call.getVideoSource() != nullptr) {
                call.getVideoSource()->restartSource();
            }

            call.setState(static_cast<TOXAV_FRIEND_CALL_STATE>(state));
        }
    }
}

// This is only a dummy implementation for now
void CoreAV::bitrateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t audioRate,
                             uint32_t videoRate, void* vSelf)
{
    auto* self = static_cast<CoreAV*>(vSelf);
    std::ignore = self;
    std::ignore = toxav;

    qDebug() << "Recommended bitrate with" << friendNum << "is now" << audioRate << "/" << videoRate
             << "- ignoring it";
}

// This is only a dummy implementation for now
void CoreAV::audioBitrateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t rate, void* vSelf)
{
    auto* self = static_cast<CoreAV*>(vSelf);
    std::ignore = self;
    std::ignore = toxav;

    qDebug() << "Recommended audio bitrate with" << friendNum << "is now" << rate << "- ignoring it";
}

// This is only a dummy implementation for now
void CoreAV::videoBitrateCallback(ToxAV* toxav, uint32_t friendNum, uint32_t rate, void* vSelf)
{
    auto* self = static_cast<CoreAV*>(vSelf);
    std::ignore = self;
    std::ignore = toxav;

    qDebug() << "Recommended video bitrate with" << friendNum << "is now" << rate << "- ignoring it";
}

void CoreAV::audioFrameCallback(ToxAV* toxAV, uint32_t friendNum, const int16_t* pcm,
                                size_t sampleCount, uint8_t channels, uint32_t samplingRate,
                                void* vSelf)
{
    std::ignore = toxAV;
    auto* self = static_cast<CoreAV*>(vSelf);
    // If call is cancelling just return
    if (self->isCancelling)
        return;
    // This callback should come from the CoreAV thread
    assert(QThread::currentThread() == self->coreAvThread.get());
    const QReadLocker locker{&self->callsLock};

    auto it = self->calls.find(friendNum);
    if (it == self->calls.end()) {
        return;
    }

    const ToxFriendCall& call = *it->second;

    if (call.getMuteVol()) {
        return;
    }

    call.playAudioBuffer(pcm, sampleCount, channels, samplingRate);
}

void CoreAV::videoFrameCallback(ToxAV* toxAV, uint32_t friendNum, uint16_t w, uint16_t h,
                                const uint8_t* y, const uint8_t* u, const uint8_t* v,
                                int32_t yStride, int32_t uStride, int32_t vStride, void* vSelf)
{
    std::ignore = toxAV;
    auto* self = static_cast<CoreAV*>(vSelf);
    // If call is cancelling just return
    if (self->isCancelling)
        return;
    // This callback should come from the CoreAV thread
    assert(QThread::currentThread() == self->coreAvThread.get());
    const QReadLocker locker{&self->callsLock};

    auto it = self->calls.find(friendNum);
    if (it == self->calls.end()) {
        return;
    }

    CoreVideoSource* videoSource = it->second->getVideoSource();
    if (videoSource == nullptr) {
        return;
    }

    vpx_image frame;
    frame.d_h = h;
    frame.d_w = w;
    frame.planes[0] = const_cast<uint8_t*>(y);
    frame.planes[1] = const_cast<uint8_t*>(u);
    frame.planes[2] = const_cast<uint8_t*>(v);
    frame.stride[0] = yStride;
    frame.stride[1] = uStride;
    frame.stride[2] = vStride;

    videoSource->pushFrame(&frame);
}
