/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "genericchatform.h"

#include "src/model/ichatlog.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/status.h"
#include "src/model/systemmessage.h"
#include "src/persistence/history.h"
#include "src/video/netcamview.h"

#include <QLabel>
#include <QSet>

class CallConfirmWidget;
class ContentDialogManager;
class Core;
class DocumentCache;
class FileTransferInstance;
class Friend;
class FriendChatState;
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
class Settings;
class SmileyPack;
class Style;

class ChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    ChatForm(Profile& profile, Friend* chatFriend, FriendChatState& chatState, IChatLog& chatLog_,
             IMessageDispatcher& messageDispatcher_, DocumentCache& documentCache,
             SmileyPack& smileyPack, CameraSource& cameraSource, Settings& settings, Style& style,
             IMessageBoxManager& messageBoxManager, ContentDialogManager& contentDialogManager,
             FriendList& friendList, ConferenceList& conferenceList, QWidget* parent = nullptr);
    ~ChatForm() override;
    void setStatusMessage(const QString& newMessage);

    void setFriendTyping(bool isTyping_);

    void show(ContentLayout* contentLayout_) final;

    static const QString ACTION_PREFIX;

signals:
    void updateFriendActivity(Friend& frnd);

public slots:
    void onAvatarChanged(const ToxPk& friendPk, const QPixmap& pic);
    void onFileNameChanged(const ToxPk& friendPk);
    void onShowMessagesClicked();
    void onSplitterMoved(int pos, int index);
    void reloadTheme() final;

private slots:
    void updateFriendActivityForFile(const ToxFile& file);
    void onAttachClicked() override;
    void onScreenshotClicked() override;

    void onFriendNameChanged(const QString& name);
    void onStatusMessage(const QString& message);
    void previewImage(const QPixmap& pixmap);
    void cancelImagePreview();
    void sendImageFromPreview();
    void doScreenshot();
    void onCopyStatusMessage();

    void callUpdateFriendActivity();

private:
    void retranslateUi();
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
    FriendChatState& chatState;
    CroppingLabel* statusMessageLabel;
    QMenu statusMessageMenu;
    QLabel* callDuration;
    QAction* copyStatusAction;
    QPixmap imagePreviewSource;
    ImagePreviewButton* imagePreview;
    std::unique_ptr<NetCamView> netcam;
    CameraSource& cameraSource;
    Settings& settings;
    Style& style;
    ContentDialogManager& contentDialogManager;
    Profile& profile;
};
