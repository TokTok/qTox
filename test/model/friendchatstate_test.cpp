/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "src/model/friendchatstate.h"

#include "src/core/icorecallcontrol.h"
#include "src/model/friend.h"
#include "src/model/ichatlog.h"
#include "src/model/systemmessage.h"
#include "src/persistence/ifriendsettings.h"
#include "src/persistence/inotificationsettings.h"

#include <QObject>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <memory>

class MockCallControl : public ICoreCallControl
{
public:
    ~MockCallControl() override;

    bool isCallStarted(const Friend* /*f*/) const override
    {
        return callStarted;
    }
    bool isCallActive(const Friend* /*f*/) const override
    {
        return callActive;
    }
    bool isCallVideoEnabled(const Friend* /*f*/) const override
    {
        return callVideoEnabled;
    }
    bool isCallInputMuted(const Friend* /*f*/) const override
    {
        return inputMuted;
    }
    bool isCallOutputMuted(const Friend* /*f*/) const override
    {
        return outputMuted;
    }

    bool startCall(uint32_t friendNum, bool video) override
    {
        std::ignore = friendNum;
        lastStartVideo = video;
        startCallCount++;
        if (startCallSucceeds) {
            callStarted = true;
            callActive = true;
            callVideoEnabled = video;
        }
        return startCallSucceeds;
    }

    bool answerCall(uint32_t friendNum, bool video) override
    {
        std::ignore = friendNum;
        lastAnswerVideo = video;
        answerCallCount++;
        if (answerCallSucceeds) {
            callActive = true;
            callVideoEnabled = video;
        }
        return answerCallSucceeds;
    }

    bool cancelCall(uint32_t friendNum) override
    {
        std::ignore = friendNum;
        cancelCallCount++;
        callStarted = false;
        callActive = false;
        return true;
    }

    void toggleMuteCallInput(const Friend* /*f*/) override
    {
        inputMuted = !inputMuted;
        toggleInputCount++;
    }

    void toggleMuteCallOutput(const Friend* /*f*/) override
    {
        outputMuted = !outputMuted;
        toggleOutputCount++;
    }

    bool callStarted = false;
    bool callActive = false;
    bool callVideoEnabled = false;
    bool inputMuted = false;
    bool outputMuted = false;

    bool startCallSucceeds = true;
    bool answerCallSucceeds = true;
    bool lastStartVideo = false;
    bool lastAnswerVideo = false;

    int startCallCount = 0;
    int answerCallCount = 0;
    int cancelCallCount = 0;
    int toggleInputCount = 0;
    int toggleOutputCount = 0;
};

MockCallControl::~MockCallControl() = default;

class MockChatLog : public IChatLog
{
    Q_OBJECT
public:
    const ChatLogItem& at(ChatLogIdx /*idx*/) const override
    {
        std::terminate();
    }

    SearchResult searchForward(SearchPos /*startIdx*/, const QString& /*phrase*/,
                               const ParameterSearch& /*parameter*/) const override
    {
        return {};
    }

    SearchResult searchBackward(SearchPos /*startIdx*/, const QString& /*phrase*/,
                                const ParameterSearch& /*parameter*/) const override
    {
        return {};
    }

    ChatLogIdx getFirstIdx() const override
    {
        return ChatLogIdx(0);
    }
    ChatLogIdx getNextIdx() const override
    {
        return ChatLogIdx(0);
    }

    std::vector<DateChatLogIdxPair> getDateIdxs(const QDate& /*startDate*/,
                                                size_t /*maxDates*/) const override
    {
        return {};
    }

    void addSystemMessage(const SystemMessage& message) override
    {
        messages.append(message);
    }

    QList<SystemMessage> messages;
};

class MockFriendSettings : public QObject, public IFriendSettings
{
    Q_OBJECT
public:
    ~MockFriendSettings() override;

    QString getContactNote(const ToxPk& /*pk*/) const override
    {
        return {};
    }
    void setContactNote(const ToxPk& /*pk*/, const QString& /*note*/) override {}
    QString getAutoAcceptDir(const ToxPk& /*pk*/) const override
    {
        return {};
    }
    void setAutoAcceptDir(const ToxPk& /*pk*/, const QString& /*dir*/) override {}
    AutoAcceptCallFlags getAutoAcceptCall(const ToxPk& /*pk*/) const override
    {
        return autoAcceptCall;
    }
    void setAutoAcceptCall(const ToxPk& /*pk*/, AutoAcceptCallFlags /*accept*/) override {}
    bool getAutoConferenceInvite(const ToxPk& /*pk*/) const override
    {
        return false;
    }
    void setAutoConferenceInvite(const ToxPk& /*pk*/, bool /*accept*/) override {}
    QString getFriendAlias(const ToxPk& /*pk*/) const override
    {
        return {};
    }
    void setFriendAlias(const ToxPk& /*pk*/, const QString& /*alias*/) override {}
    int getFriendCircleID(const ToxPk& /*pk*/) const override
    {
        return 0;
    }
    void setFriendCircleID(const ToxPk& /*pk*/, int /*circleID*/) override {}
    QDateTime getFriendActivity(const ToxPk& /*pk*/) const override
    {
        return {};
    }
    void setFriendActivity(const ToxPk& /*pk*/, const QDateTime& /*date*/) override {}
    void saveFriendSettings(const ToxPk& /*pk*/) override {}
    void removeFriendSettings(const ToxPk& /*pk*/) override {}

    SIGNAL_IMPL(MockFriendSettings, autoAcceptCallChanged, const ToxPk& pk, AutoAcceptCallFlags accept)
    SIGNAL_IMPL(MockFriendSettings, autoConferenceInviteChanged, const ToxPk& pk, bool accept)
    SIGNAL_IMPL(MockFriendSettings, autoAcceptDirChanged, const ToxPk& pk, const QString& dir)
    SIGNAL_IMPL(MockFriendSettings, contactNoteChanged, const ToxPk& pk, const QString& note)

    AutoAcceptCallFlags autoAcceptCall = AutoAcceptCall::None;
};

MockFriendSettings::~MockFriendSettings() = default;

class MockNotificationSettings : public INotificationSettings
{
public:
    ~MockNotificationSettings() override;

    bool getNotify() const override
    {
        return true;
    }
    void setNotify(bool /*newValue*/) override {}
    bool getShowWindow() const override
    {
        return false;
    }
    void setShowWindow(bool /*newValue*/) override {}
    bool getDesktopNotify() const override
    {
        return false;
    }
    void setDesktopNotify(bool /*newValue*/) override {}
    bool getNotifySystemBackend() const override
    {
        return false;
    }
    void setNotifySystemBackend(bool /*newValue*/) override {}
    bool getNotifySound() const override
    {
        return true;
    }
    void setNotifySound(bool /*newValue*/) override {}
    bool getNotifyHide() const override
    {
        return false;
    }
    void setNotifyHide(bool /*newValue*/) override {}
    bool getBusySound() const override
    {
        return false;
    }
    void setBusySound(bool /*newValue*/) override {}
    bool getConferenceAlwaysNotify() const override
    {
        return false;
    }
    void setConferenceAlwaysNotify(bool /*newValue*/) override {}
    bool getHidePostNullSuffix() const override
    {
        return false;
    }
    void setHidePostNullSuffix(bool /*newValue*/) override {}
    bool getTypingNotification() const override
    {
        return typingNotification;
    }
    void setTypingNotification(bool /*enabled*/) override {}
    bool getStatusChangeNotificationEnabled() const override
    {
        return statusChangeNotificationEnabled;
    }
    void setStatusChangeNotificationEnabled(bool /*newValue*/) override {}

    bool typingNotification = true;
    bool statusChangeNotificationEnabled = true;
};

MockNotificationSettings::~MockNotificationSettings() = default;

class TestFriendChatState : public QObject
{
    Q_OBJECT

public:
    TestFriendChatState() = default;

private slots:
    void init();

    void testIncomingCallAccept();
    void testIncomingCallReject();
    void testIncomingCallAutoAccept();
    void testOutgoingAudioCall();
    void testOutgoingVideoCall();
    void testOutgoingCallCancelWhenStarted();
    void testFriendOfflineDuringCall();
    void testMicMuteToggle();
    void testVolMuteToggle();
    void testTypingStateChanges();
    void testTypingTimerExpiry();
    void testTypingDisabledSetting();
    void testCallDuration();
    void testStatusChangeMessage();
    void testStatusChangeMessageDisabled();

private:
    std::unique_ptr<Friend> f;
    std::unique_ptr<MockCallControl> callControl;
    std::unique_ptr<MockChatLog> chatLog;
    std::unique_ptr<MockFriendSettings> friendSettings;
    std::unique_ptr<MockNotificationSettings> notificationSettings;
    std::unique_ptr<FriendChatState> state;
};

void TestFriendChatState::init()
{
    static uint8_t pk[ToxPk::size] = {1};
    f = std::make_unique<Friend>(42, ToxPk(pk));
    f->setStatus(Status::Status::Online);
    callControl = std::make_unique<MockCallControl>();
    chatLog = std::make_unique<MockChatLog>();
    friendSettings = std::make_unique<MockFriendSettings>();
    notificationSettings = std::make_unique<MockNotificationSettings>();
    state = std::make_unique<FriendChatState>(*f, *callControl, *chatLog, *friendSettings,
                                              *notificationSettings);
}

void TestFriendChatState::testIncomingCallAccept()
{
    const QSignalSpy incomingSpy(state.get(), &FriendChatState::incomingCall);
    const QSignalSpy callStartSpy(state.get(), &FriendChatState::callStarted);
    const QSignalSpy stopNotifSpy(state.get(), &FriendChatState::stopNotification);

    state->onAvInvite(42, false);

    QCOMPARE(incomingSpy.count(), 1);
    QCOMPARE(incomingSpy.at(0).at(0).toBool(), false); // video
    QCOMPARE(incomingSpy.at(0).at(1).toBool(), false); // autoAccepted
    QCOMPARE(chatLog->messages.size(), 1);
    QCOMPARE(chatLog->messages.at(0).messageType, SystemMessageType::incomingCall);

    // Now accept the call
    callControl->answerCallSucceeds = true;
    state->answerCall(false);

    QCOMPARE(stopNotifSpy.count(), 2); // once from answerCall, once from onAvStart
    QCOMPARE(callStartSpy.count(), 1);
    QCOMPARE(callControl->answerCallCount, 1);
}

void TestFriendChatState::testIncomingCallReject()
{
    const QSignalSpy rejectSpy(state.get(), &FriendChatState::rejectCallRequested);
    const QSignalSpy rejectedSpy(state.get(), &FriendChatState::callRejected);

    state->onAvInvite(42, true);
    state->rejectCall();

    QCOMPARE(rejectedSpy.count(), 1);
    QCOMPARE(rejectSpy.count(), 1);
    QCOMPARE(rejectSpy.at(0).at(0).toUInt(), 42u);
}

void TestFriendChatState::testIncomingCallAutoAccept()
{
    friendSettings->autoAcceptCall = IFriendSettings::AutoAcceptCall::Audio;

    const QSignalSpy incomingSpy(state.get(), &FriendChatState::incomingCall);
    const QSignalSpy callStartSpy(state.get(), &FriendChatState::callStarted);

    callControl->answerCallSucceeds = true;
    state->onAvInvite(42, false);

    QCOMPARE(incomingSpy.count(), 1);
    QCOMPARE(incomingSpy.at(0).at(1).toBool(), true); // autoAccepted

    // answerCall is deferred to the event loop to avoid reentrancy with toxcore
    QCOMPARE(callStartSpy.count(), 0);
    QCOMPARE(callControl->answerCallCount, 0);

    QCoreApplication::processEvents();

    QCOMPARE(callStartSpy.count(), 1);
    QCOMPARE(callControl->answerCallCount, 1);
}

void TestFriendChatState::testOutgoingAudioCall()
{
    const QSignalSpy outgoingSpy(state.get(), &FriendChatState::outgoingCall);
    const QSignalSpy outNotifSpy(state.get(), &FriendChatState::outgoingNotification);

    state->startCall(false);

    QCOMPARE(callControl->startCallCount, 1);
    QCOMPARE(callControl->lastStartVideo, false);
    QCOMPARE(outgoingSpy.count(), 1);
    QCOMPARE(outgoingSpy.at(0).at(0).toBool(), false);
    QCOMPARE(outNotifSpy.count(), 1);
    QCOMPARE(chatLog->messages.size(), 1);
    QCOMPARE(chatLog->messages.at(0).messageType, SystemMessageType::outgoingCall);
}

void TestFriendChatState::testOutgoingVideoCall()
{
    const QSignalSpy outgoingSpy(state.get(), &FriendChatState::outgoingCall);

    state->startCall(true);

    QCOMPARE(callControl->startCallCount, 1);
    QCOMPARE(callControl->lastStartVideo, true);
    QCOMPARE(outgoingSpy.count(), 1);
    QCOMPARE(outgoingSpy.at(0).at(0).toBool(), true);
}

void TestFriendChatState::testOutgoingCallCancelWhenStarted()
{
    callControl->callStarted = true;
    callControl->callVideoEnabled = true;

    state->startCall(true);

    QCOMPARE(callControl->cancelCallCount, 1);
    QCOMPARE(callControl->startCallCount, 0);
}

void TestFriendChatState::testFriendOfflineDuringCall()
{
    callControl->callStarted = true;
    callControl->callActive = true;

    const QSignalSpy stopNotifSpy(state.get(), &FriendChatState::stopNotification);
    const QSignalSpy typingSpy(state.get(), &FriendChatState::friendTypingChanged);

    f->setStatus(Status::Status::Offline);
    state->onFriendStatusChanged(f->getPublicKey(), Status::Status::Offline);

    QCOMPARE(callControl->cancelCallCount, 1);
    QCOMPARE(stopNotifSpy.count(), 1);
    QCOMPARE(typingSpy.count(), 1);
    QCOMPARE(typingSpy.at(0).at(0).toBool(), false);

    bool foundOfflineMsg = false;
    for (const auto& msg : chatLog->messages) {
        if (msg.messageType == SystemMessageType::userWentOffline) {
            foundOfflineMsg = true;
        }
    }
    QVERIFY(foundOfflineMsg);
}

void TestFriendChatState::testMicMuteToggle()
{
    callControl->callActive = true;
    const QSignalSpy micSpy(state.get(), &FriendChatState::micMuteChanged);

    state->toggleMicMute();

    QCOMPARE(callControl->toggleInputCount, 1);
    QCOMPARE(micSpy.count(), 1);
    QCOMPARE(micSpy.at(0).at(0).toBool(), true); // active
    QCOMPARE(micSpy.at(0).at(1).toBool(), true); // muted (was false, toggled)
}

void TestFriendChatState::testVolMuteToggle()
{
    callControl->callActive = true;
    const QSignalSpy volSpy(state.get(), &FriendChatState::volMuteChanged);

    state->toggleVolMute();

    QCOMPARE(callControl->toggleOutputCount, 1);
    QCOMPARE(volSpy.count(), 1);
    QCOMPARE(volSpy.at(0).at(0).toBool(), true); // active
    QCOMPARE(volSpy.at(0).at(1).toBool(), true); // muted (was false, toggled)
}

void TestFriendChatState::testTypingStateChanges()
{
    notificationSettings->typingNotification = true;

    const QSignalSpy typingSpy(state.get(), &FriendChatState::sendTypingRequested);

    // Start typing
    state->onTextChanged(true);
    QCOMPARE(typingSpy.count(), 1);
    QCOMPARE(typingSpy.at(0).at(1).toBool(), true);

    // Stop typing
    state->onTextChanged(false);
    QCOMPARE(typingSpy.count(), 2);
    QCOMPARE(typingSpy.at(1).at(1).toBool(), false);
}

void TestFriendChatState::testTypingTimerExpiry()
{
    notificationSettings->typingNotification = true;

    const QSignalSpy typingSpy(state.get(), &FriendChatState::sendTypingRequested);

    // Start typing
    state->onTextChanged(true);
    QCOMPARE(typingSpy.count(), 1);
    QCOMPARE(typingSpy.at(0).at(1).toBool(), true);

    // Wait for timer to expire (3 seconds + margin)
    QTRY_COMPARE_WITH_TIMEOUT(typingSpy.count(), 2, 4000);
    QCOMPARE(typingSpy.at(1).at(1).toBool(), false);
}

void TestFriendChatState::testTypingDisabledSetting()
{
    notificationSettings->typingNotification = false;

    const QSignalSpy typingSpy(state.get(), &FriendChatState::sendTypingRequested);

    state->onTextChanged(true);
    QCOMPARE(typingSpy.count(), 0);
}

void TestFriendChatState::testCallDuration()
{
    QSignalSpy durationSpy(state.get(), &FriendChatState::callDurationUpdated);

    callControl->callActive = true;
    state->onAvStart(42, false);

    // Duration timer should fire after ~1 second
    QTRY_VERIFY_WITH_TIMEOUT(durationSpy.count() >= 2, 2000);

    const QString text = durationSpy.last().at(0).toString();
    QVERIFY(text.contains("Call duration:") || text.isEmpty());
}

void TestFriendChatState::testStatusChangeMessage()
{
    notificationSettings->statusChangeNotificationEnabled = true;

    const QSignalSpy statusSpy(state.get(), &FriendChatState::friendStatusMessage);

    state->onFriendStatusChanged(f->getPublicKey(), Status::Status::Away);

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(statusSpy.at(0).at(1).value<SystemMessageType>(), SystemMessageType::peerStateChange);
}

void TestFriendChatState::testStatusChangeMessageDisabled()
{
    notificationSettings->statusChangeNotificationEnabled = false;

    const QSignalSpy statusSpy(state.get(), &FriendChatState::friendStatusMessage);

    state->onFriendStatusChanged(f->getPublicKey(), Status::Status::Away);

    QCOMPARE(statusSpy.count(), 0);
}

QTEST_GUILESS_MAIN(TestFriendChatState)
#include "friendchatstate_test.moc"
