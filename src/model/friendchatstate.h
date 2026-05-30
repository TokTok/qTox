/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/model/status.h"
#include "src/model/systemmessage.h"
#include "src/persistence/ifriendsettings.h"

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

class Friend;
class IChatLog;
class ICoreCallControl;
class INotificationSettings;
class ToxPk;

class FriendChatState : public QObject
{
    Q_OBJECT

public:
    FriendChatState(Friend& f, ICoreCallControl& callControl, IChatLog& chatLog,
                    IFriendSettings& friendSettings, INotificationSettings& notificationSettings,
                    QObject* parent = nullptr);

    void startCall(bool video);
    void answerCall(bool video);
    void rejectCall();
    void toggleMicMute();
    void toggleVolMute();
    void onTextChanged(bool hasText);
    void updateCallButtons();
    bool lastCallIsVideo() const;

public slots:
    void onAvInvite(uint32_t friendId, bool video);
    void onAvStart(uint32_t friendId, bool video);
    void onAvEnd(uint32_t friendId, bool error);
    void onFriendStatusChanged(const ToxPk& friendPk, Status::Status status);
    void onFriendTypingChanged(quint32 friendId, bool isTyping);

signals:
    void callStarted(bool video);
    void callEnded(bool error);
    void callRejected();
    void incomingCall(bool video, bool autoAccepted);
    void outgoingCall(bool video);
    void callButtonStateChanged(bool online, bool audio, bool video);
    void micMuteChanged(bool active, bool muted);
    void volMuteChanged(bool active, bool muted);
    void callDurationUpdated(const QString& text);
    void incomingNotification(uint32_t friendId);
    void outgoingNotification();
    void stopNotification();
    void startCallNotification(uint32_t friendId);
    void endCallNotification(uint32_t friendId);
    void rejectCallRequested(uint32_t friendId);
    void acceptCallRequested(uint32_t friendId);
    void friendTypingChanged(bool isTyping);
    void friendStatusMessage(QDateTime timestamp, SystemMessageType type, SystemMessage::Args args);
    void sendTypingRequested(uint32_t friendId, bool typing);

private:
    void startCounter();
    void stopCounter(bool error);
    void onUpdateTime();
    void emitMuteState();

    static QString secondsToDHMS(quint32 duration);

    Friend& f;
    ICoreCallControl& callControl;
    IChatLog& chatLog;
    IFriendSettings& friendSettings;
    INotificationSettings& notificationSettings;

    bool isTyping_ = false;
    bool lastCallIsVideo_ = false;
    QTimer typingTimer_;
    QTimer callDurationTimer_;
    QElapsedTimer timeElapsed_;
    bool callDurationRunning_ = false;
};
