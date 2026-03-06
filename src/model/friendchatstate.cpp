/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "friendchatstate.h"

#include "src/core/icorecallcontrol.h"
#include "src/model/friend.h"
#include "src/model/ichatlog.h"
#include "src/model/status.h"
#include "src/persistence/inotificationsettings.h"

#include <QDateTime>
#include <QDebug>

namespace {
constexpr int TYPING_NOTIFICATION_DURATION = 3000;
} // namespace

FriendChatState::FriendChatState(Friend& f_, ICoreCallControl& callControl_, IChatLog& chatLog_,
                                 IFriendSettings& friendSettings_,
                                 INotificationSettings& notificationSettings_, QObject* parent)
    : QObject(parent)
    , f(f_)
    , callControl(callControl_)
    , chatLog(chatLog_)
    , friendSettings(friendSettings_)
    , notificationSettings(notificationSettings_)
{
    typingTimer_.setSingleShot(true);
    connect(&typingTimer_, &QTimer::timeout, this, [this] {
        emit sendTypingRequested(f.getId(), false);
        isTyping_ = false;
    });

    connect(&callDurationTimer_, &QTimer::timeout, this, &FriendChatState::onUpdateTime);
}

bool FriendChatState::lastCallIsVideo() const
{
    return lastCallIsVideo_;
}

void FriendChatState::startCall(bool video)
{
    const uint32_t friendId = f.getId();
    if (callControl.isCallStarted(&f)) {
        if (!video || callControl.isCallVideoEnabled(&f)) {
            callControl.cancelCall(friendId);
        }
        return;
    }

    if (callControl.startCall(friendId, video)) {
        SystemMessage systemMessage;
        systemMessage.messageType = SystemMessageType::outgoingCall;
        systemMessage.timestamp = QDateTime::currentDateTime();
        systemMessage.args = {f.getDisplayedName()};
        chatLog.addSystemMessage(systemMessage);

        emit outgoingCall(video);
        emit outgoingNotification();
    }
}

void FriendChatState::answerCall(bool video)
{
    const uint32_t friendId = f.getId();
    emit stopNotification();
    emit acceptCallRequested(friendId);

    updateCallButtons();
    if (!callControl.answerCall(friendId, video)) {
        updateCallButtons();
        stopCounter(false);
        emit callEnded(false);
        return;
    }

    const bool actualVideo = callControl.isCallVideoEnabled(&f);
    onAvStart(friendId, actualVideo);
}

void FriendChatState::rejectCall()
{
    emit callRejected();
    emit rejectCallRequested(f.getId());
}

void FriendChatState::toggleMicMute()
{
    callControl.toggleMuteCallInput(&f);
    const bool active = callControl.isCallActive(&f);
    const bool muted = callControl.isCallInputMuted(&f);
    emit micMuteChanged(active, muted);
}

void FriendChatState::toggleVolMute()
{
    callControl.toggleMuteCallOutput(&f);
    const bool active = callControl.isCallActive(&f);
    const bool muted = callControl.isCallOutputMuted(&f);
    emit volMuteChanged(active, muted);
}

void FriendChatState::onTextChanged(bool hasText)
{
    if (!notificationSettings.getTypingNotification()) {
        if (isTyping_) {
            isTyping_ = false;
            emit sendTypingRequested(f.getId(), false);
        }
        return;
    }

    if (isTyping_ != hasText) {
        emit sendTypingRequested(f.getId(), hasText);
        if (hasText) {
            typingTimer_.start(TYPING_NOTIFICATION_DURATION);
        }
        isTyping_ = hasText;
    }
}

void FriendChatState::onAvInvite(uint32_t friendId, bool video)
{
    if (friendId != f.getId()) {
        return;
    }

    SystemMessage systemMessage;
    systemMessage.messageType = SystemMessageType::incomingCall;
    systemMessage.timestamp = QDateTime::currentDateTime();
    systemMessage.args = {f.getDisplayedName()};
    chatLog.addSystemMessage(systemMessage);

    auto testedFlag =
        video ? IFriendSettings::AutoAcceptCall::Video : IFriendSettings::AutoAcceptCall::Audio;
    if (friendSettings.getAutoAcceptCall(f.getPublicKey()).testFlag(testedFlag)) {
        qDebug() << "automatic call answer";
        // Defer the answer to the event loop to avoid reentrancy with toxcore callbacks.
        QTimer::singleShot(0, this, [this, friendId, video] {
            callControl.answerCall(friendId, video);
            onAvStart(friendId, video);
        });
        emit incomingCall(video, /*autoAccepted=*/true);
    } else {
        lastCallIsVideo_ = video;
        emit incomingCall(video, /*autoAccepted=*/false);
        emit incomingNotification(friendId);
    }
}

void FriendChatState::onAvStart(uint32_t friendId, bool video)
{
    if (friendId != f.getId()) {
        return;
    }

    emit callStarted(video);
    emit stopNotification();
    emit startCallNotification(friendId);
    updateCallButtons();
    startCounter();
}

void FriendChatState::onAvEnd(uint32_t friendId, bool error)
{
    if (friendId != f.getId()) {
        return;
    }

    emit stopNotification();
    emit endCallNotification(friendId);
    updateCallButtons();
    stopCounter(error);
    emit callEnded(error);
}

void FriendChatState::onFriendStatusChanged(const ToxPk& friendPk, Status::Status status)
{
    std::ignore = friendPk;

    if (!Status::isOnline(f.getStatus())) {
        emit friendTypingChanged(false);

        if (callControl.isCallStarted(&f)) {
            callControl.cancelCall(f.getId());
            emit stopNotification();

            SystemMessage systemMessage;
            systemMessage.messageType = SystemMessageType::userWentOffline;
            systemMessage.timestamp = QDateTime::currentDateTime();
            systemMessage.args = {f.getDisplayedName()};
            chatLog.addSystemMessage(systemMessage);
        }
    }

    updateCallButtons();

    if (notificationSettings.getStatusChangeNotificationEnabled()) {
        const QString fStatus = Status::getTitle(status);
        emit friendStatusMessage(QDateTime::currentDateTime(), SystemMessageType::peerStateChange,
                                 {f.getDisplayedName(), fStatus});
    }
}

void FriendChatState::onFriendTypingChanged(quint32 friendId, bool isTyping)
{
    if (friendId == f.getId()) {
        emit friendTypingChanged(isTyping);
    }
}

void FriendChatState::startCounter()
{
    if (callDurationRunning_) {
        return;
    }
    callDurationRunning_ = true;
    callDurationTimer_.start(1000);
    timeElapsed_.start();
    emit callDurationUpdated(secondsToDHMS(0));
}

void FriendChatState::stopCounter(bool error)
{
    if (!callDurationRunning_) {
        return;
    }

    const QString dhms = secondsToDHMS(timeElapsed_.elapsed() / 1000);
    const QString name = f.getDisplayedName();
    auto messageType = error ? SystemMessageType::unexpectedCallEnd : SystemMessageType::callEnd;

    SystemMessage systemMessage;
    systemMessage.messageType = messageType;
    systemMessage.timestamp = QDateTime::currentDateTime();
    systemMessage.args = {name, dhms};
    chatLog.addSystemMessage(systemMessage);

    callDurationTimer_.stop();
    callDurationRunning_ = false;
    emit callDurationUpdated(QString());
}

void FriendChatState::onUpdateTime()
{
    emit callDurationUpdated(secondsToDHMS(timeElapsed_.elapsed() / 1000));
}

void FriendChatState::updateCallButtons()
{
    const bool audio = callControl.isCallActive(&f);
    const bool video = callControl.isCallVideoEnabled(&f);
    const bool online = Status::isOnline(f.getStatus());
    emit callButtonStateChanged(online, audio, video);
    emitMuteState();
}

void FriendChatState::emitMuteState()
{
    const bool active = callControl.isCallActive(&f);
    emit micMuteChanged(active, callControl.isCallInputMuted(&f));
    emit volMuteChanged(active, callControl.isCallOutputMuted(&f));
}

QString FriendChatState::secondsToDHMS(quint32 duration)
{
    const QString cD = tr("Call duration: ");
    const quint32 seconds = duration % 60;
    duration /= 60;
    const quint32 minutes = duration % 60;
    duration /= 60;
    const quint32 hours = duration % 24;
    const quint32 days = duration / 24;

    if (days != 0u) {
        return cD + QString::asprintf("%ud%02uh %02um %02us", days, hours, minutes, seconds);
    }

    if (hours != 0u) {
        return cD + QString::asprintf("%02uh %02um %02us", hours, minutes, seconds);
    }

    if (minutes != 0u) {
        return cD + QString::asprintf("%02um %02us", minutes, seconds);
    }

    return cD + QString::asprintf("%02us", seconds);
}
