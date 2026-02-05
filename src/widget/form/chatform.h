/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "genericchatform.h"

#include "src/model/ichatlog.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/status.h"
#include "src/persistence/history.h"
#include "src/video/netcamview.h"

#include <QElapsedTimer>
#include <QLabel>
#include <QSet>
#include <QTimer>

class CallConfirmWidget;
class ContentDialogManager;
class Core;
class DocumentCache;
class FileTransferInstance;
class Friend;
class FriendList;
class ConferenceList;
class History;
class ImagePreviewButton;
class IMessageBoxManager;
class OfflineMsgEngine;
class Profile;
class QHideEvent;
class QMoveEvent;
class QPixmap;
class ImageLoader;
class Settings;
class SmileyPack;
class Style;

class ChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    ChatForm(Profile& profile, Friend* chatFriend, IChatLog& chatLog_,
             IMessageDispatcher& messageDispatcher_, DocumentCache& documentCache,
             SmileyPack& smileyPack, CameraSource& cameraSource, Settings& settings, Style& style,
             IMessageBoxManager& messageBoxManager, ContentDialogManager& contentDialogManager,
             FriendList& friendList, ConferenceList& conferenceList, ImageLoader& imageLoader, QWidget* parent = nullptr);
    ~ChatForm() override;
    void setStatusMessage(const QString& newMessage);

    void setFriendTyping(bool isTyping_);

    void show(ContentLayout* contentLayout_) final;

    static const QString ACTION_PREFIX;

signals:

    void incomingNotification(uint32_t friendId);
    void outgoingNotification();
    void stopNotification();
    void startCallNotification(uint32_t friendId);
    void endCallNotification(uint32_t friendId);
    void rejectCall(uint32_t friendId);
    void acceptCall(uint32_t friendId);
    void updateFriendActivity(Friend& frnd);

public slots:
    void onAvInvite(uint32_t friendId, bool video);
    void onAvStart(uint32_t friendId, bool video);
    void onAvEnd(uint32_t friendId, bool error);
    void onAvatarChanged(const ToxPk& friendPk, const QPixmap& pic);
    void onFileNameChanged(const ToxPk& friendPk);
    void onShowMessagesClicked();
    void onSplitterMoved(int pos, int index);
    void reloadTheme() final;

private slots:
    void updateFriendActivityForFile(const ToxFile& file);
    void onAttachClicked() override;
    void onScreenshotClicked() override;

    void onTextEditChanged();
    void onCallTriggered();
    void onVideoCallTriggered();
    void onAnswerCallTriggered(bool video);
    void onRejectCallTriggered();
    void onMicMuteToggle();
    void onVolMuteToggle();

    void onFriendStatusChanged(const ToxPk& friendPk, Status::Status status);
    void onFriendTypingChanged(quint32 friendId, bool isTyping_);
    void onFriendNameChanged(const QString& name);
    void onStatusMessage(const QString& message);
    void onUpdateTime();
    void previewImage(const QPixmap& pixmap);
    void cancelImagePreview();
    void sendImageFromPreview();
    void doScreenshot();
    void onCopyStatusMessage();

    void callUpdateFriendActivity();

private:
    void updateMuteMicButton();
    void updateMuteVolButton();
    void retranslateUi();
    void showOutgoingCall(bool video);
    void startCounter();
    void stopCounter(bool error = false);
    void updateCallButtons();
    void showNetcam();
    void hideNetcam();

protected:
    std::unique_ptr<NetCamView> createNetcam();
    void dragEnterEvent(QDragEnterEvent* ev) final;
    void dropEvent(QDropEvent* ev) final;
    void hideEvent(QHideEvent* event) final;
    void showEvent(QShowEvent* event) final;

private:
    Core& core;
    Friend* f;
    CroppingLabel* statusMessageLabel;
    QMenu statusMessageMenu;
    QLabel* callDuration;
    QTimer* callDurationTimer;
    QTimer typingTimer;
    QElapsedTimer timeElapsed;
    QAction* copyStatusAction;
    QPixmap imagePreviewSource;
    ImagePreviewButton* imagePreview;
    bool isTyping;
    bool lastCallIsVideo;
    std::unique_ptr<NetCamView> netcam;
    CameraSource& cameraSource;
    Settings& settings;
    Style& style;
    ContentDialogManager& contentDialogManager;
    Profile& profile;
};
