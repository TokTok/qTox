/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatform.h"
#include "src/core/toxpk.h"
#include "src/persistence/igroupsettings.h"
#include <QMap>

namespace Ui {
class MainWindow;
}
class Group;
class TabCompleter;
class FlowLayout;
class QTimer;
class GroupId;
class IMessageDispatcher;
struct Message;
class Settings;
class DocumentCache;
class SmileyPack;
class Style;
class IMessageBoxManager;
class FriendList;
class GroupList;

class GroupChatForm : public GenericChatForm
{
    Q_OBJECT
public:
    GroupChatForm(Core& core_, Group* chatGroup, IChatLog& chatLog_,
        IMessageDispatcher& messageDispatcher_, Settings& settings_,
        DocumentCache& documentCache, SmileyPack& smileyPack, Style& style,
            IMessageBoxManager& messageBoxManager, FriendList& friendList,
            GroupList& groupList);
    ~GroupChatForm();

    void peerAudioPlaying(ToxPk peerPk);

private slots:
    void onScreenshotClicked() override;
    void onAttachClicked() override;
    void onMicMuteToggle();
    void onVolMuteToggle();
    void onCallClicked();
    void onUserJoined(const ToxPk& user, const QString& name);
    void onUserLeft(const ToxPk& user, const QString& name);
    void onPeerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName);
    void onTitleChanged(const QString& author, const QString& title);
    void onLabelContextMenuRequested(const QPoint& localPos);

protected:
    void keyPressEvent(QKeyEvent* ev) final;
    void keyReleaseEvent(QKeyEvent* ev) final;
    // drag & drop
    void dragEnterEvent(QDragEnterEvent* ev) final;
    void dropEvent(QDropEvent* ev) final;

private:
    void retranslateUi();
    void updateUserCount(int numPeers);
    void updateUserNames();
    void joinGroupCall();
    void leaveGroupCall();

private:
    Core& core;
    Group* group;
    QMap<ToxPk, QLabel*> peerLabels;
    QMap<ToxPk, QTimer*> peerAudioTimers;
    FlowLayout* namesListLayout;
    QLabel* nusersLabel;
    TabCompleter* tabber;
    bool inCall;
    Settings& settings;
    Style& style;
    FriendList& friendList;
};
