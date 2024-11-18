/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "src/ipc.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <random>
#ifndef _MSC_VER
#include <unistd.h>
#endif

namespace {
#ifndef ANDROID
#ifdef Q_OS_WIN
const char* getCurUsername()
{
    return getenv("USERNAME");
}
#else
const char* getCurUsername()
{
    return getenv("USER");
}
#endif

QString getIpcKey()
{
    auto* user = getCurUsername();
    if (user == nullptr) {
        qWarning() << "Failed to get current username. Will use a global IPC.";
        user = "";
    }
    return QString("qtox-" IPC_PROTOCOL_VERSION "-") + QString::fromUtf8(user);
}
#endif
} // namespace

/**
 * @var time_t IPC::lastEvent
 * @brief When last event was posted.
 *
 * @var time_t IPC::lastProcessed
 * @brief When processEvents() ran last time
 */

/**
 * @class IPC
 * @brief Inter-process communication
 */

IPC::IPC(uint32_t profileId_)
    : profileId{profileId_}
#ifndef ANDROID
    , globalMemory{getIpcKey()}
#endif
{
    qRegisterMetaType<IPCEventHandler>("IPCEventHandler");

    timer.setInterval(EVENT_TIMER_MS);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, &IPC::processEvents);

    // The first started instance gets to manage the shared memory by taking ownership
    // Every time it processes events it updates the global shared timestamp "lastProcessed"
    // If the timestamp isn't updated, that's a timeout and someone else can take ownership
    // This is a safety measure, in case one of the clients crashes
    // If the owner exits normally, it can set the timestamp to 0 first to immediately give
    // ownership

    // use the clock rather than std::random_device because std::random_device may return constant values, and does
    // under mingw on Windows. We don't actually need cryptographic guarantees, so using the clock in all cases.
    static std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<uint64_t> distribution;
    globalId = distribution(rng);
    qDebug() << "Our global IPC ID is" << globalId;
#ifdef ANDROID
    return;
#else
    if (globalMemory.create(sizeof(IPCMemory))) {
        if (globalMemory.lock()) {
            IPCMemory* mem = global();
            memset(mem, 0, sizeof(IPCMemory));
            mem->globalId = globalId;
            mem->lastProcessed = time(nullptr);
            globalMemory.unlock();
        } else {
            qWarning() << "Couldn't lock to take ownership";
        }
    } else if (globalMemory.attach()) {
        qDebug() << "Attaching to the global shared memory";
    } else {
        qDebug() << "Failed to attach to the global shared memory, giving up. Error:"
                 << globalMemory.error();
        return; // We won't be able to do any IPC without being attached, let's get outta here
    }
#endif

    processEvents();
}

IPC::~IPC()
{
#ifndef ANDROID
    if (!globalMemory.lock()) {
        qWarning() << "Failed to lock in ~IPC";
        return;
    }

    if (isCurrentOwnerNoLock()) {
        global()->globalId = 0;
    }
    globalMemory.unlock();
#endif
}

/**
 * @brief Post IPC event.
 * @param name Name to set in IPC event.
 * @param data Data to set in IPC event (default QByteArray()).
 * @param dest Settings::getCurrentProfileId() or 0 (main instance, default).
 * @return Time the event finished or 0 on error.
 */
time_t IPC::postEvent(const QString& name, const QByteArray& data, uint32_t dest)
{
    const QByteArray binName = name.toUtf8();
    if (binName.length() > static_cast<int32_t>(sizeof(IPCEvent::name))) {
        return 0;
    }

    if (data.length() > static_cast<int32_t>(sizeof(IPCEvent::data))) {
        return 0;
    }

#ifdef ANDROID
    std::ignore = dest;
    return 0;
#else
    if (!globalMemory.lock()) {
        qDebug() << "Failed to lock in postEvent()";
        return 0;
    }

    IPCEvent* evt = nullptr;
    IPCMemory* mem = global();
    time_t result = 0;

    for (uint32_t i = 0; (evt == nullptr) && i < EVENT_QUEUE_SIZE; ++i) {
        if (mem->events[i].posted == 0) {
            evt = &mem->events[i];
        }
    }

    if (evt != nullptr) {
        memset(evt, 0, sizeof(IPCEvent));
        memcpy(evt->name, binName.constData(), binName.length());
        memcpy(evt->data, data.constData(), data.length());
        mem->lastEvent = evt->posted = result = qMax(mem->lastEvent + 1, time(nullptr));
        evt->dest = dest;
        evt->sender = getpid();
        qDebug() << "postEvent" << name << "to" << dest;
    }

    globalMemory.unlock();
    return result;
#endif
}

bool IPC::isCurrentOwner()
{
#ifdef ANDROID
    return false;
#else
    if (globalMemory.lock()) {
        const bool isOwner = isCurrentOwnerNoLock();
        globalMemory.unlock();
        return isOwner;
    }
    qWarning() << "isCurrentOwner failed to lock, returning false";
    return false;

#endif
}

/**
 * @brief Register a handler for an IPC event
 * @param handler The handler callback. Should not block for more than a second, at worst
 */
void IPC::registerEventHandler(const QString& name, IPCEventHandler handler, void* userData)
{
    const std::lock_guard<std::mutex> lock(eventHandlersMutex);
    eventHandlers[name] = {handler, userData};
}

void IPC::unregisterEventHandler(const QString& name)
{
    const std::lock_guard<std::mutex> lock(eventHandlersMutex);
    eventHandlers.remove(name);
}

bool IPC::isEventAccepted(time_t time)
{
#ifdef ANDROID
    std::ignore = time;
    return false;
#else
    bool result = false;
    if (!globalMemory.lock()) {
        return result;
    }

    if (difftime(global()->lastProcessed, time) > 0) {
        IPCMemory* mem = global();
        for (auto& event : mem->events) {
            if (event.posted == time && (event.processed != 0)) {
                result = event.accepted;
                break;
            }
        }
    }
    globalMemory.unlock();

    return result;
#endif
}

bool IPC::waitUntilAccepted(time_t postTime, int32_t timeout /*=-1*/)
{
    bool result = false;
    const time_t start = time(nullptr);
    forever
    {
        result = isEventAccepted(postTime);
        if (result || (timeout > 0 && difftime(time(nullptr), start) >= timeout)) {
            break;
        }

        qApp->processEvents();
        QThread::msleep(0);
    }
    return result;
}

bool IPC::isAttached() const
{
#ifdef ANDROID
    return false;
#else
    return globalMemory.isAttached();
#endif
}

void IPC::setProfileId(uint32_t profileId_)
{
    profileId = profileId_;
}

/**
 * @brief Only called when global memory IS LOCKED.
 * @return nullptr if no events present, IPC event otherwise
 */
IPC::IPCEvent* IPC::fetchEvent()
{
    IPCMemory* mem = global();
    for (auto& event : mem->events) {
        IPCEvent* evt = &event;

        // Garbage-collect events that were not processed in EVENT_GC_TIMEOUT
        // and events that were processed and EVENT_GC_TIMEOUT passed after
        // so sending instance has time to react to those events.
        if (((evt->processed != 0) && difftime(time(nullptr), evt->processed) > EVENT_GC_TIMEOUT)
            || ((evt->processed == 0) && difftime(time(nullptr), evt->posted) > EVENT_GC_TIMEOUT)) {
            memset(evt, 0, sizeof(IPCEvent));
        }

        if ((evt->posted != 0) && (evt->processed == 0) && evt->sender != getpid()
            && (evt->dest == profileId || (evt->dest == 0 && isCurrentOwnerNoLock()))) {
            return evt;
        }
    }

    return nullptr;
}

bool IPC::runEventHandler(IPCEventHandler handler, const QByteArray& arg, void* userData)
{
    bool result = false;
    if (QThread::currentThread() == qApp->thread()) {
        result = handler(arg, userData);
    } else {
        QMetaObject::invokeMethod(this, "runEventHandler", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, result), Q_ARG(IPCEventHandler, handler),
                                  Q_ARG(const QByteArray&, arg));
    }

    return result;
}

void IPC::processEvents()
{
#ifdef ANDROID
    timer.start();
    return;
#else
    if (!globalMemory.lock()) {
        timer.start();
        return;
    }

    IPCMemory* mem = global();

    if (mem->globalId == globalId) {
        // We're the owner, let's process those events
        mem->lastProcessed = time(nullptr);
    } else {
        // Only the owner processes events. But if the previous owner's dead, we can take
        // ownership now
        if (difftime(time(nullptr), mem->lastProcessed) >= OWNERSHIP_TIMEOUT_S) {
            qDebug() << "Previous owner timed out, taking ownership" << mem->globalId << "->"
                     << globalId;
            // Ignore events that were not meant for this instance
            memset(mem, 0, sizeof(IPCMemory));
            mem->globalId = globalId;
            mem->lastProcessed = time(nullptr);
        }
        // Non-main instance is limited to events destined for specific profile it runs
    }

    const std::lock_guard<std::mutex> lock(eventHandlersMutex);
    while (IPCEvent* evt = fetchEvent()) {
        const QString name = QString::fromUtf8(evt->name);
        auto it = eventHandlers.find(name);
        if (it != eventHandlers.end()) {
            evt->accepted = runEventHandler(it.value().handler, evt->data, it.value().userData);
            qDebug() << "Processed event:" << name << "posted:" << evt->posted
                     << "accepted:" << evt->accepted;
            if (evt->dest == 0) {
                // Global events should be processed only by instance that accepted event.
                // Otherwise global
                // event would be consumed by very first instance that gets to check it.
                if (evt->accepted) {
                    evt->processed = time(nullptr);
                }
            } else {
                evt->processed = time(nullptr);
            }
        } else {
            qDebug() << "Received event:" << name << "without handler";
            qDebug() << "Available handlers:" << eventHandlers.keys();
        }
    }

    globalMemory.unlock();
    timer.start();
#endif
}

/**
 * @brief Only called when global memory IS LOCKED.
 * @return true if owner, false if not owner or if error
 */
bool IPC::isCurrentOwnerNoLock()
{
#ifdef ANDROID
    return false;
#else
    const void* const data = globalMemory.data();
    if (data == nullptr) {
        qWarning() << "isCurrentOwnerNoLock failed to access the memory, returning false";
        return false;
    }
    return (*static_cast<const uint64_t*>(data) == globalId);
#endif
}

IPC::IPCMemory* IPC::global()
{
#ifdef ANDROID
    return nullptr;
#else
    return static_cast<IPCMemory*>(globalMemory.data());
#endif
}
