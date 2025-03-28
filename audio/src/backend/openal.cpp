/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "openal.h"

#include "audio/iaudiosettings.h"

#include <QDebug>
#include <QFile>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QPointer>
#include <QThread>
#include <QWaitCondition>
#include <QtMath>

#include <cassert>

#if defined(QT_STATIC) && !defined(Q_OS_WASM)
extern "C"
{
    typedef void alsoftLogCallback(void* userptr, char level, const char* message, int length) noexcept;
    /**
     * @brief Set a callback for OpenAL logging.
     *
     * @note This function is only available in statically linked builds where we know for sure we
     * link against openal-soft.
     */
    void alsoft_set_log_callback(alsoftLogCallback* callback, void* userptr) noexcept;
}
#endif

namespace {
void applyGain(int16_t* buffer, uint32_t bufferSize, qreal gainFactor)
{
    for (quint32 i = 0; i < bufferSize; ++i) {
        // gain amplification with clipping to 16-bit boundaries
        buffer[i] =
            qBound<int16_t>(std::numeric_limits<int16_t>::min(), qRound(buffer[i] * gainFactor),
                            std::numeric_limits<int16_t>::max());
    }
}

/**
 * @class OpenAL
 * @brief Provides the OpenAL audio backend
 *
 * @var BUFFER_COUNT
 * @brief Number of buffers to use per audio source
 *
 * @var AUDIO_CHANNELS
 * @brief Ideally, we'd auto-detect, but that's a sane default
 */

constexpr unsigned int BUFFER_COUNT = 16;
constexpr uint32_t AUDIO_CHANNELS = 2;

namespace logcat {
#if defined(QT_STATIC) && !defined(Q_OS_WASM)
Q_LOGGING_CATEGORY(openal, "openal")
#endif
Q_LOGGING_CATEGORY(audio, "qtox.audio")
} // namespace logcat
} // namespace

OpenAL::OpenAL(IAudioSettings& _settings)
    : settings{_settings}
    , audioThread{new QThread}
{
#if defined(QT_STATIC) && !defined(Q_OS_WASM)
    alsoft_set_log_callback(
        [](void* userptr, char level, const char* message, int length) noexcept {
            std::ignore = userptr;
            // OpenAL passes .data() and .size() to the callback,
            // so we can't rely on null-termination.
            const std::string_view msg{message, static_cast<size_t>(length)};
            switch (level) {
            case 'E':
                qCCritical(logcat::openal).noquote() << msg;
                break;
            case 'W':
                qCWarning(logcat::openal).noquote() << msg;
                break;
            case 'I':
                qCDebug(logcat::openal).noquote() << msg;
                break;
            default:
                // Shouldn't happen, but if it does, we'll log it as a warning.
                qCWarning(logcat::openal).noquote() << level << msg;
                break;
            }
        },
        nullptr);
#endif

    // initialize OpenAL error stack
    alGetError();
    alcGetError(nullptr);

    audioThread->setObjectName("qTox Audio");
    QObject::connect(audioThread, &QThread::finished, &voiceTimer, &QTimer::stop);
    QObject::connect(audioThread, &QThread::finished, &captureTimer, &QTimer::stop);
    QObject::connect(audioThread, &QThread::finished, audioThread, &QThread::deleteLater);

    moveToThread(audioThread);

    voiceTimer.setSingleShot(true);
    voiceTimer.moveToThread(audioThread);
    connect(this, &OpenAL::startActive, this, [this](qreal time) { voiceTimer.start(time); });
    connect(&voiceTimer, &QTimer::timeout, this, &OpenAL::stopActive);

    connect(&captureTimer, &QTimer::timeout, this, &OpenAL::doAudio);
    captureTimer.setInterval(AUDIO_FRAME_DURATION / 2);
    captureTimer.setSingleShot(false);
    captureTimer.moveToThread(audioThread);
    // TODO for Qt 5.6+: use qOverload
    connect(audioThread, &QThread::started, &captureTimer,
            static_cast<void (QTimer::*)(void)>(&QTimer::start));

    cleanupTimer.setInterval(1000);
    cleanupTimer.setSingleShot(false);
    connect(&cleanupTimer, &QTimer::timeout, this, &OpenAL::cleanupSound);
    // TODO for Qt 5.6+: use qOverload
    connect(audioThread, &QThread::started, &cleanupTimer,
            static_cast<void (QTimer::*)(void)>(&QTimer::start));

    audioThread->start();
}

OpenAL::~OpenAL()
{
    audioThread->exit();
    audioThread->wait();
    cleanupInput();
    cleanupOutput();
}

void OpenAL::checkAlError() noexcept
{
    const ALenum al_err = alGetError();
    if (al_err != AL_NO_ERROR) {
        qWarning("OpenAL error: %s", alGetString(al_err));
    }
}

void OpenAL::checkAlcError(ALCdevice* device) noexcept
{
    const ALCenum alc_err = alcGetError(device);
    if (alc_err != 0) {
        qWarning("OpenAL error: %s", alcGetString(device, alc_err));
    }
}

/**
 * @brief Set the master output volume.
 *
 * @param[in] volume   the master volume (between 0 and 1)
 */
void OpenAL::setOutputVolume(qreal volume)
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    alListenerf(AL_GAIN, static_cast<ALfloat>(std::max(0.0, std::min(volume, 1.0))));
    checkAlError();
}

/**
 * @brief The minimum gain value for an input device.
 *
 * @return minimum gain value in dB
 */
qreal OpenAL::minInputGain() const
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);
    return minInGain;
}

/**
 * @brief The maximum gain value for an input device.
 *
 * @return maximum gain value in dB
 */
qreal OpenAL::maxInputGain() const
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);
    return maxInGain;
}

/**
 * @brief The minimum threshold value for an input device.
 *
 * @return minimum normalized threshold
 */
qreal OpenAL::minInputThreshold() const
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);
    return minInThreshold;
}

/**
 * @brief The maximum normalized threshold value for an input device.
 *
 * @return maximum normalized threshold
 */
qreal OpenAL::maxInputThreshold() const
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);
    return maxInThreshold;
}

void OpenAL::reinitInput(const QString& inDevDesc)
{
    QMutexLocker<QRecursiveMutex> locker(&audioLock);

    const auto bakSources = sources;
    sources.clear();

    cleanupInput();
    initInput(inDevDesc);

    locker.unlock();
    // this must happen outside `audioLock`, to avoid a deadlock when
    // a slot on AlSource::invalidate tries to create a new source immediately.
    for (const auto& source : bakSources) {
        source->kill();
    }
}

bool OpenAL::reinitOutput(const QString& outDevDesc)
{
    QMutexLocker<QRecursiveMutex> locker(&audioLock);

    const auto bakSinks = sinks;

    sinks.clear();

    cleanupOutput();
    const bool result = initOutput(outDevDesc);

    locker.unlock();
    // this must happen outside `audioLock`, to avoid a deadlock when
    // a slot on AlSink::invalidate tries to create a new source immediately.
    for (const auto& sink : bakSinks) {
        sink->kill();
    }

    return result;
}

/**
 * @brief Allocates ressources for a new audio output
 * @return AudioSink on success, nullptr on failure
 */
std::unique_ptr<IAudioSink> OpenAL::makeSink()
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    if (!autoInitOutput()) {
        qWarning("Failed to subscribe to audio output device.");
        return {};
    }

    ALuint sid;
    alGenSources(1, &sid);

    auto* const sink = new AlSink(*this, sid);
    if (sink == nullptr) {
        return {};
    }

    sinks.insert(sink);
    qCDebug(logcat::audio) << "Audio source" << sid << "created. Sources active:" << sinks.size();

    return std::unique_ptr<IAudioSink>{sink};
}

/**
 * @brief Must be called by the destructor of AlSink to remove the internal ressources.
 *        If no sinks are opened, the output is closed afterwards.
 * @param sink Audio sink to remove.
 */
void OpenAL::destroySink(AlSink& sink)
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    const auto sinksErased = sinks.erase(&sink);
    const auto soundSinksErased = soundSinks.erase(&sink);

    if (sinksErased == 0 && soundSinksErased == 0) {
        qCWarning(logcat::audio) << "Destroying non-existent sink";
        return;
    }

    const uint sid = sink.getSourceId();

    if (alIsSource(sid) != 0) {
        // stop playing, marks all buffers as processed
        alSourceStop(sid);
        cleanupBuffers(sid);
        qCDebug(logcat::audio) << "Audio source" << sid << "deleted. Sources active:" << sinks.size();
    } else {
        qCWarning(logcat::audio) << "Trying to delete invalid audio source" << sid;
    }

    if (sinks.empty() && soundSinks.empty()) {
        cleanupOutput();
    }
}

/**
 * @brief Subscribe to capture sound from the opened input device.
 *
 * If the input device is not open, it will be opened before capturing.
 */
std::unique_ptr<IAudioSource> OpenAL::makeSource()
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    if (!autoInitInput()) {
        qWarning("Failed to subscribe to audio input device.");
        return {};
    }

    auto* const source = new AlSource(*this);
    sources.insert(source);

    qCDebug(logcat::audio) << "Subscribed to audio input device [" << sources.size()
                           << "subscriptions ]";

    return std::unique_ptr<IAudioSource>{source};
}

/**
 * @brief Unsubscribe from capturing from an opened input device.
 *
 * If the input device has no more subscriptions, it will be closed.
 */
void OpenAL::destroySource(AlSource& source)
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    const auto s = sources.find(&source);
    if (s == sources.end()) {
        qCWarning(logcat::audio) << "Destroyed non-existent source";
        return;
    }

    sources.erase(s);

    qCDebug(logcat::audio) << "Unsubscribed from audio input device [" << sources.size()
                           << "subscriptions left ]";

    if (sources.empty()) {
        cleanupInput();
    }
}

/**
 * @brief Initialize audio input device, if not initialized.
 *
 * @return true, if device was initialized; false otherwise
 */
bool OpenAL::autoInitInput()
{
    return (alInDev != nullptr) ? true : initInput(settings.getInDev());
}

/**
 * @brief Initialize audio output device, if not initialized.
 *
 * @return true, if device was initialized; false otherwise
 */
bool OpenAL::autoInitOutput()
{
    return (alOutDev != nullptr) ? true : initOutput(settings.getOutDev());
}

bool OpenAL::initInput(const QString& deviceName)
{
    return initInput(deviceName, AUDIO_CHANNELS);
}

bool OpenAL::initInput(const QString& deviceName, uint32_t channels)
{
    if (!settings.getAudioInDevEnabled()) {
        return false;
    }

    qCDebug(logcat::audio) << "Opening audio input" << deviceName;
    assert(!alInDev);

    // TODO: Try to actually detect if our audio source is stereo
    inputChannels = channels;
    const int stereoFlag = inputChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    const int bytesPerSample = 2;
    const int safetyFactor = 2; // internal OpenAL ring buffer. must be larger than our inputBuffer
                                // to avoid the ring from overwriting itself between captures.
    AUDIO_FRAME_SAMPLE_COUNT_TOTAL = AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL * inputChannels;
    const ALCsizei ringBufSize = AUDIO_FRAME_SAMPLE_COUNT_TOTAL * bytesPerSample * safetyFactor;

    const QByteArray qDevName = deviceName.toUtf8();
    const ALchar* tmpDevName = qDevName.isEmpty() ? nullptr : qDevName.constData();
    alInDev = alcCaptureOpenDevice(tmpDevName, AUDIO_SAMPLE_RATE, stereoFlag, ringBufSize);

    // Restart the capture if necessary
    if (alInDev == nullptr) {
        qCWarning(logcat::audio) << "Failed to initialize audio input device:" << deviceName;
        return false;
    }

    inputBuffer = new int16_t[AUDIO_FRAME_SAMPLE_COUNT_TOTAL];
    setInputGain(settings.getAudioInGainDecibel());
    setInputThreshold(settings.getAudioThreshold());

    qCDebug(logcat::audio) << "Opened audio input" << deviceName;
    alcCaptureStart(alInDev);

    return true;
}

/**
 * @brief Open an audio output device
 */
bool OpenAL::initOutput(const QString& deviceName)
{
    // there should be no sinks when initializing the output
    assert(sinks.empty());

    outputInitialized = false;
    if (!settings.getAudioOutDevEnabled())
        return false;

    qCDebug(logcat::audio) << "Opening audio output" << deviceName;
    assert(!alOutDev);

    const QByteArray qDevName = deviceName.toUtf8();
    const ALchar* tmpDevName = qDevName.isEmpty() ? nullptr : qDevName.constData();
    alOutDev = alcOpenDevice(tmpDevName);

    if (alOutDev == nullptr) {
        qCWarning(logcat::audio) << "Cannot open audio output device" << deviceName;
        return false;
    }

    qCDebug(logcat::audio) << "Opened audio output" << deviceName;
    alOutContext = alcCreateContext(alOutDev, nullptr);
    checkAlcError(alOutDev);

    if (alcMakeContextCurrent(alOutContext) == 0) {
        qCWarning(logcat::audio) << "Cannot create audio output context";
        return false;
    }

    // init master volume
    alListenerf(AL_GAIN, settings.getOutVolume() * 0.01f);
    checkAlError();

    outputInitialized = true;
    return true;
}

/**
 * @brief Play a 48kHz mono 16bit PCM sound
 */
void OpenAL::playMono16Sound(AlSink& sink, const IAudioSink::Sound& sound)
{
    const uint sourceId = sink.getSourceId();
    QFile sndFile(IAudioSink::getSound(sound));
    if (!sndFile.exists()) {
        qCDebug(logcat::audio) << "Trying to open non-existent sound file";
        return;
    }

    sndFile.open(QIODevice::ReadOnly);
    const QByteArray data{sndFile.readAll()};
    if (data.isEmpty()) {
        qCDebug(logcat::audio) << "Sound file contained no data";
        return;
    }

    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    // interrupt possibly playing sound, we don't buffer here
    alSourceStop(sourceId);

    ALint processed = 0;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    alSourcei(sourceId, AL_LOOPING, AL_FALSE);

    ALuint bufid;
    if (processed == 0) {
        // create new buffer
        alGenBuffers(1, &bufid);
    } else {
        // we only reserve space for one buffer
        assert(processed == 1);
        // unqueue all processed buffers
        alSourceUnqueueBuffers(sourceId, processed, &bufid);
    }

    alBufferData(bufid, AL_FORMAT_MONO16, data.constData(), data.size(), AUDIO_SAMPLE_RATE);
    alSourcei(sourceId, AL_BUFFER, bufid);
    alSourcePlay(sourceId);
    soundSinks.insert(&sink);
}

void OpenAL::cleanupSound()
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    auto sinkIt = soundSinks.begin();
    while (sinkIt != soundSinks.end()) {
        auto* sink = *sinkIt;
        const ALuint sourceId = sink->getSourceId();
        ALint state = 0;

        alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING) {
            sinkIt = soundSinks.erase(sinkIt);
            emit sink->finishedPlaying();
        } else {
            ++sinkIt;
        }
    }
}

void OpenAL::playAudioBuffer(uint sourceId, const int16_t* data, int samples, unsigned channels,
                             int sampleRate)
{
    assert(channels == 1 || channels == 2);
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);

    if ((alOutDev == nullptr) || !outputInitialized)
        return;

    ALuint bufids[BUFFER_COUNT];
    ALint processed = 0;
    ALint queued = 0;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queued);
    alSourcei(sourceId, AL_LOOPING, AL_FALSE);

    if (processed == 0) {
        if (static_cast<ALuint>(queued) >= BUFFER_COUNT) {
            // reached limit, drop audio
            return;
        }
        // create new buffer if none got free and we're below the limit
        alGenBuffers(1, bufids);
    } else {
        // unqueue all processed buffers
        alSourceUnqueueBuffers(sourceId, processed, bufids);
        // delete all but the first buffer, reuse first for new data
        alDeleteBuffers(processed - 1, bufids + 1);
    }

    alBufferData(bufids[0], (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data,
                 samples * 2 * channels, sampleRate);
    alSourceQueueBuffers(sourceId, 1, bufids);

    ALint state;
    alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) {
        alSourcePlay(sourceId);
    }
}

/**
 * @brief Close active audio input device.
 */
void OpenAL::cleanupInput()
{
    if (alInDev == nullptr)
        return;

    qCDebug(logcat::audio) << "Closing audio input";
    alcCaptureStop(alInDev);
    if (alcCaptureCloseDevice(alInDev) == ALC_TRUE) {
        alInDev = nullptr;
    } else {
        qCWarning(logcat::audio) << "Failed to close input";
    }

    delete[] inputBuffer;
}

/**
 * @brief Close active audio output device
 */
void OpenAL::cleanupOutput()
{
    outputInitialized = false;

    if (alOutDev != nullptr) {
        if (alcMakeContextCurrent(nullptr) == 0) {
            qWarning("Failed to clear audio context");
        }

        alcDestroyContext(alOutContext);
        alOutContext = nullptr;

        qCDebug(logcat::audio) << "Closing audio output";
        if (alcCloseDevice(alOutDev) != 0) {
            alOutDev = nullptr;
        } else {
            qWarning("Failed to close output");
        }
    }
}

/**
 * @brief Called by doCapture to calculate volume of the audio buffer
 *
 * @param[in] buf   the current audio buffer
 *
 * @return normalized volume between 0-1
 */
qreal OpenAL::getVolume()
{
    const quint32 samples = AUDIO_FRAME_SAMPLE_COUNT_TOTAL;
    constexpr qreal sqrt2 = 1.41421356237; // std::numbers::sqrt2
    // calculate volume as the root mean squared of amplitudes in the sample
    qreal sumOfSquares = 0;
    for (quint32 i = 0; i < samples; i++) {
        auto sample = static_cast<qreal>(inputBuffer[i]) / std::numeric_limits<int16_t>::max();
        sumOfSquares += std::pow(sample, 2);
    }
    const qreal rms = std::sqrt(sumOfSquares / samples);
    // our calculated normalized volume could possibly be above 1 because our RMS assumes a sinusoidal wave
    const qreal normalizedVolume = std::min(rms * sqrt2, 1.0);
    return normalizedVolume;
}

/**
 * @brief Called by voiceTimer's timeout to disable audio broadcasting
 */
void OpenAL::stopActive()
{
    isActive = false;
}

/**
 * @brief handles recording of audio frames
 */
void OpenAL::doInput()
{
    ALint curSamples = 0;
    alcGetIntegerv(alInDev, ALC_CAPTURE_SAMPLES, sizeof(curSamples), &curSamples);
    if (curSamples < static_cast<ALint>(AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL)) {
        return;
    }

    captureSamples(alInDev, inputBuffer, AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL);

    applyGain(inputBuffer, AUDIO_FRAME_SAMPLE_COUNT_TOTAL, gainFactor);

    auto volume = getVolume();
    if (volume >= inputThreshold) {
        isActive = true;
        emit startActive(voiceHold);
    } else if (!isActive) {
        volume = 0;
    }

    // NOTE(sudden6): this loop probably doesn't scale too well with many sources
    for (auto* source : sources) {
        emit source->volumeAvailable(volume);
    }
    if (!isActive) {
        return;
    }

    // NOTE(sudden6): this loop probably doesn't scale too well with many sources
    for (auto* source : sources) {
        emit source->frameAvailable(inputBuffer, AUDIO_FRAME_SAMPLE_COUNT_PER_CHANNEL,
                                    inputChannels, AUDIO_SAMPLE_RATE);
    }
}

void OpenAL::doOutput()
{
    // Nothing
}

/**
 * @brief Called on the captureTimer events to capture audio
 */
void OpenAL::doAudio()
{
    const QMutexLocker<QRecursiveMutex> lock(&audioLock);

    // Output section does nothing

    // Input section
    if ((alInDev != nullptr) && !sources.empty()) {
        doInput();
    }
}

void OpenAL::captureSamples(ALCdevice* device, int16_t* buffer, ALCsizei samples)
{
    alcCaptureSamples(device, buffer, samples);
}

/**
 * @brief Returns true if the output device is open
 */
bool OpenAL::isOutputReady() const
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);
    return (alOutDev != nullptr) && outputInitialized;
}

QStringList OpenAL::outDeviceNames()
{
    QStringList list;
    const ALchar* pDeviceList = (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") != AL_FALSE)
                                    ? alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER)
                                    : alcGetString(nullptr, ALC_DEVICE_SPECIFIER);

    if (pDeviceList != nullptr) {
        while (*pDeviceList != 0) {
            auto len = strlen(pDeviceList);
            list << QString::fromUtf8(pDeviceList, len);
            pDeviceList += len + 1;
        }
    }

    return list;
}

QStringList OpenAL::inDeviceNames()
{
    QStringList list;
    const ALchar* pDeviceList = alcGetString(nullptr, ALC_CAPTURE_DEVICE_SPECIFIER);

    if (pDeviceList != nullptr) {
        qCDebug(logcat::audio) << "Available input devices:" << pDeviceList;
        while (*pDeviceList != 0) {
            auto len = strlen(pDeviceList);
            list << QString::fromUtf8(pDeviceList, len);
            pDeviceList += len + 1;
        }
    }

    return list;
}

/**
 * @brief Free all buffers that finished playing on a source
 * @param sourceId where to remove the buffers from
 */
void OpenAL::cleanupBuffers(uint sourceId)
{
    // unqueue all buffers from the source
    ALint processed = 0;
    alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    std::vector<ALuint> bufids;
    // should never be out of range, just to be sure
    assert(processed >= 0);
    bufids.resize(processed);
    alSourceUnqueueBuffers(sourceId, processed, bufids.data());
    // delete all buffers
    alDeleteBuffers(processed, bufids.data());
}

void OpenAL::startLoop(uint sourceId)
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);
    alSourcei(sourceId, AL_LOOPING, AL_TRUE);
}

void OpenAL::stopLoop(uint sourceId)
{
    const QMutexLocker<QRecursiveMutex> locker(&audioLock);
    alSourcei(sourceId, AL_LOOPING, AL_FALSE);
    alSourceStop(sourceId);
    cleanupBuffers(sourceId);
}

qreal OpenAL::inputGain() const
{
    return gain;
}

qreal OpenAL::getInputThreshold() const
{
    return inputThreshold;
}

qreal OpenAL::inputGainFactor() const
{
    return gainFactor;
}

void OpenAL::setInputGain(qreal dB)
{
    gain = qBound(minInGain, dB, maxInGain);
    gainFactor = qPow(10.0, (gain / 20.0));
}

void OpenAL::setInputThreshold(qreal normalizedThreshold)
{
    inputThreshold = normalizedThreshold;
}
