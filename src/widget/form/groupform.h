/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "genericchatform.h"

#include "src/core/icoregroupquery.h"
#include "src/core/toxpk.h"

#include <QMap>

namespace Ui {
class MainWindow;
}
class Group;
class FlowLayout;
class QTimer;
class IMessageDispatcher;
class GroupMessageDispatcher;
struct Message;
class Settings;
class DocumentCache;
class SmileyPack;
class Style;
class IMessageBoxManager;
class FriendList;
class ConferenceList;
class GroupList;
class CroppingLabel;
class QLabel;
class QToolButton;
class QWidget;

class GroupForm : public GenericChatForm
{
    Q_OBJECT
public:
    GroupForm(Core& core_, Group* chatGroup, IChatLog& chatLog_,
              IMessageDispatcher& messageDispatcher_, Settings& settings_,
              DocumentCache& documentCache, SmileyPack& smileyPack, Style& style,
              IMessageBoxManager& messageBoxManager, FriendList& friendList,
              ConferenceList& conferenceList, GroupList& groupList);
    ~GroupForm() override;

signals:

private slots:
    void onScreenshotClicked() override;
    void onAttachClicked() override;
    void onSendTriggered() override;
    void onUserJoined(const ToxPk& user, const QString& name);
    void onUserLeft(const ToxPk& user, const QString& name);
    void onPeerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName);
    void onPeerStatusChanged(const ToxPk& peer, Status::Status status);
    void onTitleChanged(const QString& author, const QString& title);
    void onTopicChanged(const QString& author, const QString& topic);
    void onLabelContextMenuRequested(const QPoint& localPos);
    void onTopicContextMenuRequested(const QPoint& localPos);
    void editTopic();
    void setPassword();
    void setNickname();
    void clearPassword();
    void setPeerLimit();
    void startPrivateMessage(const ToxPk& peerPk);
    void cancelPrivateMessage();

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
    void updateTopicLabel();
    static QString roleIcon(GroupRole role);
    bool canSetTopic() const;
    void updatePrivateMessageIndicator();

private:
    Core& core;
    Group* group;
    GroupMessageDispatcher* groupDispatcher;
    QMap<ToxPk, QLabel*> peerLabels;
    FlowLayout* namesListLayout;
    QLabel* nusersLabel;
    CroppingLabel* topicLabel;
    Settings& settings;
    Style& style;
    FriendList& friendList;
    ToxPk privateMessageTarget;
    QWidget* privateMessageBar;
    QLabel* privateMessageLabel;
    QToolButton* privateMessageCloseButton;
};
