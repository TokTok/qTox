/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "genericchatitemwidget.h"
#include "ui_mainwindow.h"

#include "audio/iaudiocontrol.h"
#include "audio/iaudiosink.h"
#include "src/core/conferenceid.h"
#include "src/core/toxfile.h"
#include "src/core/toxid.h"
#include "src/core/toxpk.h"
#include "src/model/notificationgenerator.h"
#include "src/platform/desktop_notifications/desktopnotify.h"
#include "src/widget/form/debugwidget.h"

#include <QFileInfo>
#include <QMainWindow>
#include <QPointer>
#include <QSystemTrayIcon>
#include <QUrl>

#include <memory>

namespace Ui {
class MainWindow;
}

class AddFriendForm;
class ChatManager;
class AlSink;
class Camera;
class ChatForm;
class CircleWidget;
class ContentDialog;
class ContentLayout;
class Core;
class FilesForm;
class Friend;
class FriendChatroom;
class FriendChatState;
class FriendListWidget;
class FriendWidget;
class GenericChatroomWidget;
class Conference;
class ConferenceForm;
class ConferenceRoom;
class ConferenceInvite;
class ConferenceInviteForm;
class ConferenceWidget;
class ConferenceMessageDispatcher;
class DocumentCache;
class FriendMessageDispatcher;
class MaskablePixmapWidget;
class ProfileForm;
class ProfileInfo;
class QActionGroup;
class QMenu;
class QPushButton;
class QSplitter;
class QTimer;
class SettingsWidget;
class SystemTrayIcon;
class VideoSurface;
class UpdateCheck;
class Settings;
class IChatLog;
class ChatHistory;
class SmileyPack;
class CameraSource;
class Style;
class IMessageBoxManager;
class ContentDialogManager;
class FriendList;
class ConferenceList;
class IPC;
class ToxSave;
class Nexus;

class Widget final : public QMainWindow
{
    Q_OBJECT

private:
    enum class ActiveToolMenuButton
    {
        AddButton,
        ConferenceButton,
        TransferButton,
        SettingButton,
        DebugButton,
        None,
    };

    enum class DialogType
    {
        AddDialog,
        TransferDialog,
        SettingDialog,
        ProfileDialog,
        ConferenceDialog,
        DebugDialog,
    };

    enum class FilterCriteria
    {
        All = 0,
        Online,
        Offline,
        Friends,
        Conferences
    };

public:
    Widget(Profile& profile_, IAudioControl& audio_, CameraSource& cameraSource, Settings& settings,
           Style& style, IPC& ipc, Nexus& nexus, QWidget* parent = nullptr);
    ~Widget() override;
    void init();
    void setCentralWidget(QWidget* widget, const QString& widgetName);
    QString getUsername();
    Camera* getCamera();
    static Widget* getInstance(IAudioControl* audio = nullptr);
    void showUpdateDownloadProgress();
    void addFriendDialog(const Friend* frnd, ContentDialog* dialog);
    void addConferenceDialog(const Conference* conference, ContentDialog* dialog);
    bool newFriendMessageAlert(const ToxPk& friendId, const QString& text, bool sound = true,
                               QString filename = QString(), size_t filesize = 0);
    bool newConferenceMessageAlert(const ConferenceId& conferenceId, const ToxPk& authorPk,
                                   const QString& message, bool notify);
    bool getIsWindowMinimized();
    void updateIcons();

    static QString fromDialogType(DialogType type);
    ContentDialog* createContentDialog() const;
    ContentLayout* createContentDialog(DialogType type) const;

    void clearAllReceipts();

    static inline QIcon prepareIcon(QString path, int w = 0, int h = 0);

    bool conferencesVisible() const;

    void resetIcon();
    void registerIpcHandlers();
    static bool toxActivateEventHandler(const QByteArray& data, void* userData);
    bool handleToxSave(const QString& path);

public slots:
    void reloadTheme();
    void onShowSettings();
    void onShowDebug();
    void onSeparateWindowClicked(bool separate);
    void onSeparateWindowChanged(bool separate, bool clicked);
    void setWindowTitle(const QString& title);
    void forceShow();
    void onConnected();
    void onDisconnected();
    void onStatusSet(Status::Status status);
    void onFailedToStartCore();
    void onBadProxyCore();
    void onSelfAvatarLoaded(const QPixmap& pic);
    void setUsername(const QString& username);
    void setStatusMessage(const QString& statusMessage);
    void addFriendFailed(const ToxPk& userId, const QString& errorInfo = QString());
    void onFriendStatusChanged(const ToxPk& friendPk, Status::Status status);
    void onFriendStatusMessageUpdated(const ToxPk& friendPk, const QString& message);
    void onFriendDisplayedNameChanged(const QString& displayed);
    void onFriendAliasChanged(const ToxPk& friendId, const QString& alias);
    void onFriendRequestReceived(const ToxPk& friendPk, const QString& message);
    void onFileReceiveRequested(const ToxFile& file);
    void onConferenceInviteReceived(const ConferenceInvite& inviteInfo);
    void onConferenceInviteAccepted(const ConferenceInvite& inviteInfo);
    void titleChangedByUser(const QString& title);
    void onConferencePeerAudioPlaying(uint32_t conferencenumber, ToxPk peerPk);
    void onConferenceSendFailed(uint32_t conferencenumber);
    void onFriendTypingChanged(uint32_t friendNumber, bool isTyping);
    void nextChat();
    void previousChat();
    void onFriendDialogShown(const Friend* f);
    void onConferenceDialogShown(Conference* c);
    void toggleFullScreen();
    void refreshPeerListsLocal(const QString& username);
    void onUpdateAvailable();
    void onCoreChanged(Core& core);

signals:
    void friendRequestAccepted(const ToxPk& friendPk);
    void friendRequested(const ToxId& friendAddress, const QString& message);
    void statusSet(Status::Status status);
    void statusSelected(Status::Status status);
    void usernameChanged(const QString& username);
    void changeConferenceTitle(uint32_t conferencenumber, const QString& title);
    void statusMessageChanged(const QString& statusMessage);
    void resized();
    void windowStateChanged(Qt::WindowStates states);

private slots:
    void onAddClicked();
    void onConferenceClicked();
    void onTransferClicked();
    void showProfile();
    void openNewDialog(GenericChatroomWidget* widget);
    void onChatroomWidgetClicked(GenericChatroomWidget* widget);
    void onStatusMessageChanged(const QString& newStatusMessage);
    void removeFriend(const ToxPk& friendId);
    void copyFriendIdToClipboard(const ToxPk& friendId);
    void removeConference(const ConferenceId& conferenceId);
    void setStatusOnline();
    void setStatusAway();
    void setStatusBusy();
    void onIconClick(QSystemTrayIcon::ActivationReason reason);
    void onUserAwayCheck();
    void onEventIconTick();
    void onTryCreateTrayIcon();
    void onEnableDebugChanged(bool newValue);
    void onSetShowSystemTray(bool newValue);
    void onSplitterMoved(int pos, int index);
    void friendListContextMenu(const QPoint& pos);
    void friendRequestsUpdate();
    void conferenceInvitesUpdate();
    void conferenceInvitesClear();
    void onStartConferenceCall(uint32_t conferenceId);
    void onEndConferenceCall(uint32_t conferenceId);
    void onDialogShown(GenericChatroomWidget* widget);
    void outgoingNotification();
    void onCallStart(uint32_t friendId);
    void onCallEnd(uint32_t friendId);
    void incomingNotification(uint32_t friendNum);
    void onRejectCall(uint32_t friendId);
    void onStopNotification();
    void dispatchFile(ToxFile file);
    void dispatchFileWithBool(ToxFile file, bool pausedOrBroken);
    void dispatchFileSendFailed(uint32_t friendId, const QString& fileName);
    void connectCircleWidget(CircleWidget& circleWidget) const;
    void connectFriendWidget(FriendWidget& friendWidget) const;
    void searchCircle(CircleWidget& circleWidget);
    void updateFriendActivity(const Friend& frnd);
    void registerContentDialog(ContentDialog& contentDialog) const;

    void onFriendModelAdded(Friend* newFriend, std::shared_ptr<FriendChatroom> chatroom,
                            std::shared_ptr<FriendMessageDispatcher> dispatcher,
                            std::shared_ptr<ChatHistory> chatHistory, FriendChatState* chatState);
    void onConferenceModelAdded(Conference* newConference, std::shared_ptr<ConferenceRoom> chatroom,
                                std::shared_ptr<ConferenceMessageDispatcher> dispatcher,
                                std::shared_ptr<IChatLog> chatHistory);
    void onConferenceNeedsName(const ConferenceId& conferenceId);

private:
    // QMainWindow overrides
    bool eventFilter(QObject* obj, QEvent* event) final;
    bool event(QEvent* e) final;
    void closeEvent(QCloseEvent* event) final;
    void changeEvent(QEvent* event) final;
    void resizeEvent(QResizeEvent* event) final;
    void moveEvent(QMoveEvent* event) final;

    bool newMessageAlert(QWidget* currentWindow, bool isActive, bool sound = true, bool notify = true);
    void setActiveToolMenuButton(ActiveToolMenuButton newActiveButton);
    void hideMainForms(GenericChatroomWidget* chatroomWidget);
    void removeFriend(Friend* f, bool fake = false);
    void removeConference(Conference* c, bool fake = false);
    void saveWindowGeometry();
    void saveSplitterGeometry();
    void cycleChats(bool forward);
    void searchChats();
    void changeDisplayMode();
    void updateFilterText();
    FilterCriteria getFilterCriteria() const;
    static bool filterGroups(FilterCriteria index);
    static bool filterOnline(FilterCriteria index);
    static bool filterOffline(FilterCriteria index);
    void retranslateUi();
    void focusChatInput();
    void openDialog(GenericChatroomWidget* widget, bool newWindow);
    void playNotificationSound(IAudioSink::Sound sound, bool loop = false);
    void cleanupNotificationSound();
    void acceptFileTransfer(const ToxFile& file, const QString& path);
    void formatWindowTitle(const QString& content);

private:
    Profile& profile;
    std::unique_ptr<QSystemTrayIcon> icon;
    QMenu* trayMenu;
    QAction* statusOnline;
    QAction* statusAway;
    QAction* statusBusy;
    QAction* actionLogout;
    QAction* actionQuit;
    QAction* actionShow;

    QMenu* filterMenu;

    QActionGroup* filterGroup;
    QAction* filterAllAction;
    QAction* filterOnlineAction;
    QAction* filterOfflineAction;
    QAction* filterFriendsAction;
    QAction* filterGroupsAction;

    QActionGroup* filterDisplayGroup;
    QAction* filterDisplayName;
    QAction* filterDisplayActivity;

    Ui::MainWindow* ui;
    QSplitter* centralLayout;
    QPoint dragPosition;
    ContentLayout* contentLayout;
    AddFriendForm* addFriendForm;
    ConferenceInviteForm* conferenceInviteForm;

    ProfileInfo* profileInfo;
    ProfileForm* profileForm;

    QPointer<DebugWidget> debugWidget;
    QPointer<SettingsWidget> settingsWidget;
    std::unique_ptr<UpdateCheck> updateCheck; // ownership should be moved outside Widget once non-singleton
    FilesForm* filesForm;
    static Widget* instance;
    GenericChatroomWidget* activeChatroomWidget;
    FriendListWidget* chatListWidget;
    MaskablePixmapWidget* profilePicture;
    bool notify(QObject* receiver, QEvent* event);
    bool autoAwayActive = false;
    QTimer* timer;
    bool eventFlag;
    bool eventIcon;
    bool wasMaximized = false;
    QPushButton* friendRequestsButton;
    QPushButton* conferenceInvitesButton;
    unsigned int unreadConferenceInvites;
    int icon_size;

    IAudioControl& audio;
    std::unique_ptr<IAudioSink> audioNotification;
    Settings& settings;

    QMap<ToxPk, FriendWidget*> friendWidgets;
    // Stop gap method of linking our friend messages back to a conference id.
    // Eventual goal is to have a notification manager that works on
    // Messages hooked up to message dispatchers but we aren't there
    // yet
    QMap<ToxPk, QMetaObject::Connection> friendAlertConnections;
    QMap<ToxPk, ChatForm*> chatForms;

    QMap<ConferenceId, ConferenceWidget*> conferenceWidgets;
    // Stop gap method of linking our conference messages back to a conference id.
    // Eventual goal is to have a notification manager that works on
    // Messages hooked up to message dispatchers but we aren't there
    // yet
    QMap<ConferenceId, QMetaObject::Connection> conferenceAlertConnections;
    QMap<ConferenceId, QSharedPointer<ConferenceForm>> conferenceForms;
    Core* core = nullptr;

    std::unique_ptr<ChatManager> chatManager;
    std::unique_ptr<NotificationGenerator> notificationGenerator;
    std::unique_ptr<DesktopNotify> notifier;

#ifdef Q_OS_MAC
    QAction* fileMenu;
    QAction* editMenu;
    QAction* contactMenu;
    QMenu* changeStatusMenu;
    QAction* editProfileAction;
    QAction* logoutAction;
    QAction* addContactAction;
    QAction* nextConversationAction;
    QAction* previousConversationAction;
#endif
    std::unique_ptr<SmileyPack> smileyPack;
    std::unique_ptr<DocumentCache> documentCache;
    CameraSource& cameraSource;
    Style& style;
    IMessageBoxManager* messageBoxManager = nullptr; // freed by Qt on destruction
    std::unique_ptr<FriendList> friendList;
    std::unique_ptr<ConferenceList> conferenceList;
    std::unique_ptr<ContentDialogManager> contentDialogManager;
    IPC& ipc;
    std::unique_ptr<ToxSave> toxSave;
    Nexus& nexus;
};
