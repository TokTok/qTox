/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "groupform.h"

#include "src/chatlog/chatwidget.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/model/groupmessagedispatcher.h"
#include "src/persistence/settings.h"
#include "src/widget/chatformheader.h"
#include "src/widget/flowlayout.h"
#include "src/widget/form/chatform.h"
#include "src/widget/style.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/translator.h"

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextDocument>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
const auto LABEL_PEER_TYPE_OUR = QVariant(QStringLiteral("our"));
const auto LABEL_PEER_TYPE_MUTED = QVariant(QStringLiteral("muted"));
const auto PEER_LABEL_STYLE_SHEET_PATH = QStringLiteral("chatArea/chatHead.qss");

/**
 * @brief Edit name for correct representation if it is needed
 * @param name Editing string
 * @return Source name if it does not contain any newline character, otherwise it chops characters
 * starting with first newline character and appends "..."
 */
QString editName(const QString& name)
{
    const int pos = name.indexOf(QRegularExpression(QStringLiteral("[\n\r]")));
    if (pos == -1) {
        return name;
    }

    QString result = name;
    const int len = result.length();
    result.chop(len - pos);
    result.append(QStringLiteral("…")); // \u2026 Unicode symbol, not just three separate dots
    return result;
}
} // namespace

GroupForm::GroupForm(Core& core_, Group* chatGroup, IChatLog& chatLog_,
                     IMessageDispatcher& messageDispatcher_, Settings& settings_,
                     DocumentCache& documentCache_, SmileyPack& smileyPack_, Style& style_,
                     IMessageBoxManager& messageBoxManager, FriendList& friendList_,
                     ConferenceList& conferenceList_, GroupList& groupList_)
    : GenericChatForm(core_, chatGroup, chatLog_, messageDispatcher_, documentCache_,
                      smileyPack_, settings_, style_, messageBoxManager, friendList_,
                      conferenceList_, groupList_)
    , core{core_}
    , group(chatGroup)
    , groupDispatcher(qobject_cast<GroupMessageDispatcher*>(&messageDispatcher_))
    , settings(settings_)
    , style{style_}
    , friendList{friendList_}
{
    nusersLabel = new QLabel();

    fileButton->setEnabled(false);
    fileButton->setProperty("state", "");
    headWidget->setMode(ChatFormHeader::Mode::None);
    headWidget->setNameEditable(false);
    setName(group->getDisplayedName());

    nusersLabel->setFont(Style::getFont(Style::Font::Medium));
    nusersLabel->setObjectName("statusLabel");

    const QSize& size = headWidget->getAvatarSize();
    headWidget->setAvatar(Style::scaleSvgImage(":/img/group_dark.svg", size.width(), size.height()));

    msgEdit->setObjectName("conference");

    namesListLayout = new FlowLayout(0, 5, 0);
    headWidget->addWidget(nusersLabel);
    headWidget->addLayout(namesListLayout);

    topicLabel = new CroppingLabel(this);
    topicLabel->setObjectName("topicLabel");
    topicLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    headWidget->addWidget(topicLabel);
    updateTopicLabel();

    headWidget->addStretch();

    nusersLabel->setMinimumHeight(12);

    privateMessageBar = new QWidget(this);
    privateMessageBar->setObjectName("privateMessageBar");
    auto* privateMessageLayout = new QHBoxLayout(privateMessageBar);
    privateMessageLayout->setContentsMargins(8, 4, 8, 4);
    privateMessageLayout->setSpacing(8);

    privateMessageLabel = new QLabel(privateMessageBar);
    privateMessageLabel->setObjectName("privateMessageLabel");
    privateMessageLayout->addWidget(privateMessageLabel);
    privateMessageLayout->addStretch();

    privateMessageCloseButton = new QToolButton(privateMessageBar);
    privateMessageCloseButton->setObjectName("privateMessageCloseButton");
    privateMessageCloseButton->setAutoRaise(true);
    privateMessageCloseButton->setIcon(QIcon::fromTheme("dialog-close", QIcon(":/img/close.svg")));
    privateMessageLayout->addWidget(privateMessageCloseButton);

    privateMessageBar->hide();
    contentLayout->insertWidget(contentLayout->count() - 1, privateMessageBar);

    connect(privateMessageCloseButton, &QToolButton::clicked, this, &GroupForm::cancelPrivateMessage);
    connect(msgEdit, &ChatTextEdit::escapePressed, this, &GroupForm::cancelPrivateMessage);

    connect(headWidget, &ChatFormHeader::nameChanged, chatGroup, &Group::setName);
    connect(group, &Group::titleChanged, this, &GroupForm::onTitleChanged);
    connect(group, &Group::topicChanged, this, &GroupForm::onTopicChanged);
    connect(group, &Group::userJoined, this, &GroupForm::onUserJoined);
    connect(group, &Group::userLeft, this, &GroupForm::onUserLeft);
    connect(group, &Group::peerNameChanged, this, &GroupForm::onPeerNameChanged);
    connect(group, &Group::peerStatusChanged, this, &GroupForm::onPeerStatusChanged);
    connect(group, &Group::numPeersChanged, this, &GroupForm::updateUserCount);
    connect(group, &Group::peerRolesChanged, this, &GroupForm::updateUserNames);
    connect(topicLabel, &CroppingLabel::customContextMenuRequested, this,
            &GroupForm::onTopicContextMenuRequested);
    connect(topicLabel, &CroppingLabel::clicked, this, &GroupForm::editTopic);
    settings.connectTo_blockListChanged(this, [this](const QStringList&) { updateUserNames(); });

    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::selfJoinedConference, {});
    }

    updateUserNames();
    retranslateUi();
    setAcceptDrops(true);
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

GroupForm::~GroupForm()
{
    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::selfLeftConference, {});
    }
    Translator::unregister(this);
}

QString GroupForm::roleIcon(GroupRole role)
{
    switch (role) {
    case GroupRole::Founder:
        return QStringLiteral("<font color=\"#FFD700\">★</font> ");
    case GroupRole::Moderator:
        return QStringLiteral("<font color=\"#FFD700\">☆</font> ");
    case GroupRole::Observer:
        return QStringLiteral("<font color=\"#9E9E9E\">◉</font> ");
    default:
        return {};
    }
}

void GroupForm::onTitleChanged(const QString& author, const QString& title)
{
    if (author.isEmpty()) {
        return;
    }

    const QDateTime curTime = QDateTime::currentDateTime();
    addSystemInfoMessage(curTime, SystemMessageType::titleChanged, {author, title});
}

void GroupForm::onTopicChanged(const QString& author, const QString& topic)
{
    updateTopicLabel();

    if (author.isEmpty()) {
        return;
    }

    const QDateTime curTime = QDateTime::currentDateTime();
    addSystemInfoMessage(curTime, SystemMessageType::titleChanged, {author, topic});
}

void GroupForm::updateTopicLabel()
{
    const QString topic = group->getTopic();
    const bool empty = topic.isEmpty();
    topicLabel->setText(empty ? tr("No topic") : topic);
    topicLabel->setProperty("empty", empty);
    Style::repolish(topicLabel);
}

void GroupForm::onScreenshotClicked()
{
    // Unsupported
}

void GroupForm::onAttachClicked()
{
    // Unsupported
}

/**
 * @brief Updates user names' labels at the top of the group
 */
void GroupForm::updateUserNames()
{
    QLayoutItem* child;
    while ((child = namesListLayout->takeAt(0)) != nullptr) {
        child->widget()->hide();
        delete child->widget();
        delete child;
    }

    peerLabels.clear();
    const auto peers = group->getPeerList();

    // no need to do anything without any peers
    if (peers.isEmpty()) {
        return;
    }

    /* we store the peer labels by their ToxPk, but the namelist layout
     * needs it in alphabetical order, so we first create and store the labels
     * and then sort them by their text and add them to the layout in that order */
    const auto selfPk = core.getGroupSelfPk(group->getId());
    for (const auto& peerPk : peers.keys()) {
        const QString peerName = peers.value(peerPk);
        const QString editedName = editName(peerName);
        const QString roleIconStr = roleIcon(group->getPeerRole(peerPk));
        const Status::Status status = group->getPeerStatus(peerPk);
        const QString statusIcon = QString("<img src='%1' width='12' height='12'/> ").arg(Status::getIconPath(status));
        QLabel* label;
        if (roleIconStr.isEmpty()) {
            label = new QLabel(statusIcon + editedName.toHtmlEscaped() + QLatin1String(", "));
            label->setTextFormat(Qt::RichText);
        } else {
            label = new QLabel(statusIcon + roleIconStr + editedName.toHtmlEscaped() + QLatin1String(", "));
            label->setTextFormat(Qt::RichText);
        }
        label->setProperty("peerSortName", editedName.toLower());
        if (editedName != peerName) {
            label->setToolTip(peerName + " (" + peerPk.toString() + ")");
        } else if (peerName != peerPk.toString()) {
            label->setToolTip(peerPk.toString());
        } // else their name is just their Pk, no tooltip needed
        label->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(label, &QLabel::customContextMenuRequested, this,
                &GroupForm::onLabelContextMenuRequested);

        if (peerPk == selfPk) {
            label->setProperty("peerType", LABEL_PEER_TYPE_OUR);
        } else if (settings.getBlockList().contains(peerPk.toString())) {
            label->setProperty("peerType", LABEL_PEER_TYPE_MUTED);
        }

        label->setStyleSheet(style.getStylesheet(PEER_LABEL_STYLE_SHEET_PATH, settings));
        peerLabels.insert(peerPk, label);
    }

    // add the labels in alphabetical order into the layout
    auto nickLabelList = peerLabels.values();

    std::sort(nickLabelList.begin(), nickLabelList.end(), [](const QLabel* a, const QLabel* b) {
        return a->property("peerSortName").toString() < b->property("peerSortName").toString();
    });

    // remove comma from last sorted label
    QLabel* const lastLabel = nickLabelList.last();
    QString labelText = lastLabel->text();
    labelText.chop(2);
    lastLabel->setText(labelText);
    for (QLabel* l : nickLabelList) {
        namesListLayout->addWidget(l);
    }
}

void GroupForm::onUserJoined(const ToxPk& user, const QString& name)
{
    std::ignore = user;
    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::userJoinedConference,
                             {name});
    }
    updateUserNames();
}

void GroupForm::onUserLeft(const ToxPk& user, const QString& name)
{
    std::ignore = user;
    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::userLeftConference,
                             {name});
    }
    updateUserNames();
}

void GroupForm::onPeerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName)
{
    std::ignore = peer;
    addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::peerNameChanged,
                         {oldName, newName});
    updateUserNames();
}

void GroupForm::onPeerStatusChanged(const ToxPk& peer, Status::Status status)
{
    std::ignore = peer;
    std::ignore = status;
    updateUserNames();
}

void GroupForm::dragEnterEvent(QDragEnterEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk toxPk{ev->mimeData()->data("toxPk")};
    Friend* frnd = friendList.findFriend(toxPk);
    if (frnd != nullptr) {
        ev->acceptProposedAction();
    }
}

void GroupForm::dropEvent(QDropEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk toxPk{ev->mimeData()->data("toxPk")};
    Friend* frnd = friendList.findFriend(toxPk);
    if (frnd == nullptr) {
        return;
    }

    const uint32_t friendId = frnd->getId();
    const uint32_t groupNumber = group->getId();
    if (Status::isOnline(frnd->getStatus())) {
        core.groupInviteFriend(friendId, groupNumber);
    }
}

void GroupForm::keyPressEvent(QKeyEvent* ev)
{
    std::ignore = ev;
    if (msgEdit->hasFocus()) {
        return;
    }
}

void GroupForm::keyReleaseEvent(QKeyEvent* ev)
{
    std::ignore = ev;
    if (msgEdit->hasFocus()) {
        return;
    }
}

/**
 * @brief Updates users' count label text
 */
void GroupForm::updateUserCount(int numPeers)
{
    nusersLabel->setText(tr("%n user(s) in chat", "Number of users in chat", numPeers));
}

void GroupForm::retranslateUi()
{
    updateUserCount(group->getPeersCount());
    updatePrivateMessageIndicator();
}

void GroupForm::onLabelContextMenuRequested(const QPoint& localPos)
{
    auto* label = static_cast<QLabel*>(QObject::sender());

    if (label == nullptr) {
        return;
    }

    const QPoint pos = label->mapToGlobal(localPos);
    const QString muteString = tr("mute");
    const QString unmuteString = tr("unmute");
    QStringList blockList = settings.getBlockList();
    auto* const contextMenu = new QMenu(this);
    const ToxPk selfPk = core.getGroupSelfPk(group->getId());
    ToxPk peerPk;

    // delete menu after it stops being used
    connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QObject::deleteLater);

    peerPk = peerLabels.key(label);
    if (peerPk.isEmpty() || peerPk == selfPk) {
        return;
    }

    const bool isPeerBlocked = blockList.contains(peerPk.toString());
    QString menuTitle = label->text();
    if (menuTitle.endsWith(QLatin1String(", "))) {
        menuTitle.chop(2);
    }

    // remove HTML tags from the title, if any, so that it's displayed correctly in the menu
    QTextDocument doc;
    doc.setHtml(menuTitle);
    menuTitle = doc.toPlainText();

    QAction* menuTitleAction = contextMenu->addAction(menuTitle);
    menuTitleAction->setEnabled(false); // make sure the title is not clickable
    contextMenu->addSeparator();

    const QAction* toggleMuteAction;
    if (isPeerBlocked) {
        toggleMuteAction = contextMenu->addAction(unmuteString);
    } else {
        toggleMuteAction = contextMenu->addAction(muteString);
    }
    contextMenu->setStyleSheet(style.getStylesheet(PEER_LABEL_STYLE_SHEET_PATH, settings));

    auto* copyIdAction = contextMenu->addAction(tr("copy peer ID"));

    const GroupRole selfRole = group->getPeerRole(selfPk);
    const GroupRole peerRole = group->getPeerRole(peerPk);
    QAction* promoteAction = nullptr;
    QAction* demoteAction = nullptr;
    QAction* kickAction = nullptr;
    if (selfRole == GroupRole::Founder || selfRole == GroupRole::Moderator) {
        if (peerRole == GroupRole::User || peerRole == GroupRole::Observer) {
            promoteAction = contextMenu->addAction(tr("promote to moderator"));
        }
        if (peerRole == GroupRole::Moderator) {
            demoteAction = contextMenu->addAction(tr("demote to user"));
        }
        kickAction = contextMenu->addAction(tr("kick from group"));
    }

    auto* privateMessageAction = contextMenu->addAction(tr("private message"));
    contextMenu->addSeparator();

    const QAction* selectedItem = contextMenu->exec(pos);
    if (selectedItem == nullptr) {
        return;
    }
    if (selectedItem == toggleMuteAction) {
        if (isPeerBlocked) {
            const int index = blockList.indexOf(peerPk.toString());
            if (index != -1) {
                blockList.removeAt(index);
            }
        } else {
            blockList << peerPk.toString();
        }

        settings.setBlockList(blockList);
    } else if (selectedItem == copyIdAction) {
        auto* clipboard = QApplication::clipboard();
        clipboard->setText(peerPk.toString(), QClipboard::Clipboard);
        if (clipboard->supportsSelection()) {
            clipboard->setText(peerPk.toString(), QClipboard::Selection);
        }
    } else if (selectedItem == promoteAction) {
        group->setPeerRole(peerPk, GroupRole::Moderator);
    } else if (selectedItem == demoteAction) {
        group->setPeerRole(peerPk, GroupRole::User);
    } else if (selectedItem == kickAction) {
        group->kickPeer(peerPk);
    } else if (selectedItem == privateMessageAction) {
        startPrivateMessage(peerPk);
    }
}

void GroupForm::onTopicContextMenuRequested(const QPoint& localPos)
{
    const QPoint pos = topicLabel->mapToGlobal(localPos);
    auto* const contextMenu = new QMenu(this);
    contextMenu->setStyleSheet(style.getStylesheet("chatArea/chatHead.qss", settings));

    connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QObject::deleteLater);

    auto* copyTopicAction = contextMenu->addAction(tr("Copy topic"));
    auto* copyIdAction = contextMenu->addAction(tr("Copy group ID"));
    const GroupRole selfRole = group->getPeerRole(core.getGroupSelfPk(group->getId()));
    QAction* setTopicAction = nullptr;
    QAction* setNicknameAction = nullptr;
    QMenu* statusMenu = nullptr;
    QAction* statusOnlineAction = nullptr;
    QAction* statusAwayAction = nullptr;
    QAction* statusBusyAction = nullptr;
    if (canSetTopic()) {
        setTopicAction = contextMenu->addAction(tr("Set topic..."));
    }
    setNicknameAction = contextMenu->addAction(tr("Set nickname..."));
    
    statusMenu = contextMenu->addMenu(tr("My status"));
    const Status::Status currentStatus = group->getGroupStatus();
    statusOnlineAction = statusMenu->addAction(QIcon(Status::getIconPath(Status::Status::Online)), tr("Online"));
    statusOnlineAction->setCheckable(true);
    statusOnlineAction->setChecked(currentStatus == Status::Status::Online);
    statusAwayAction = statusMenu->addAction(QIcon(Status::getIconPath(Status::Status::Away)), tr("Away"));
    statusAwayAction->setCheckable(true);
    statusAwayAction->setChecked(currentStatus == Status::Status::Away);
    statusBusyAction = statusMenu->addAction(QIcon(Status::getIconPath(Status::Status::Busy)), tr("Busy"));
    statusBusyAction->setCheckable(true);
    statusBusyAction->setChecked(currentStatus == Status::Status::Busy);

    QAction* setPasswordAction = nullptr;
    QAction* clearPasswordAction = nullptr;
    QAction* setPeerLimitAction = nullptr;
    QAction* setTopicLockAction = nullptr;
    QMenu* voiceStateMenu = nullptr;
    QMenu* privacyStateMenu = nullptr;
    QAction* voiceAllAction = nullptr;
    QAction* voiceModeratorAction = nullptr;
    QAction* voiceFounderAction = nullptr;
    QAction* privacyPublicAction = nullptr;
    QAction* privacyPrivateAction = nullptr;
    if (selfRole == GroupRole::Founder) {
        contextMenu->addSeparator();
        if (group->isPasswordSet()) {
            clearPasswordAction = contextMenu->addAction(tr("Remove group password"));
        } else {
            setPasswordAction = contextMenu->addAction(tr("Set group password..."));
        }
        setPeerLimitAction = contextMenu->addAction(tr("Set peer limit..."));
        setTopicLockAction = contextMenu->addAction(tr("Lock topic"));
        setTopicLockAction->setCheckable(true);
        setTopicLockAction->setChecked(group->getTopicLock() == GroupTopicLock::Enabled);

        voiceStateMenu = contextMenu->addMenu(tr("Who can speak"));
        const GroupVoiceState voiceState = group->getVoiceState();
        voiceAllAction = voiceStateMenu->addAction(tr("Everyone"));
        voiceAllAction->setCheckable(true);
        voiceAllAction->setChecked(voiceState == GroupVoiceState::All);
        voiceModeratorAction = voiceStateMenu->addAction(tr("Moderators and Founder"));
        voiceModeratorAction->setCheckable(true);
        voiceModeratorAction->setChecked(voiceState == GroupVoiceState::Moderator);
        voiceFounderAction = voiceStateMenu->addAction(tr("Only Founder"));
        voiceFounderAction->setCheckable(true);
        voiceFounderAction->setChecked(voiceState == GroupVoiceState::Founder);

        privacyStateMenu = contextMenu->addMenu(tr("Group visibility"));
        const GroupPrivacyState privacyState = group->getPrivacyState();
        privacyPublicAction = privacyStateMenu->addAction(tr("Public (join by link)"));
        privacyPublicAction->setCheckable(true);
        privacyPublicAction->setChecked(privacyState == GroupPrivacyState::Public);
        privacyPrivateAction = privacyStateMenu->addAction(tr("Private (invite only)"));
        privacyPrivateAction->setCheckable(true);
        privacyPrivateAction->setChecked(privacyState == GroupPrivacyState::Private);
    }

    const QAction* selectedItem = contextMenu->exec(pos);
    if (selectedItem == nullptr) {
        return;
    }
    if (selectedItem == copyTopicAction) {
        auto* clipboard = QApplication::clipboard();
        clipboard->setText(group->getTopic(), QClipboard::Clipboard);
        if (clipboard->supportsSelection()) {
            clipboard->setText(group->getTopic(), QClipboard::Selection);
        }
    } else if (selectedItem == copyIdAction) {
        auto* clipboard = QApplication::clipboard();
        clipboard->setText(group->getPersistentId().toString(), QClipboard::Clipboard);
        if (clipboard->supportsSelection()) {
            clipboard->setText(group->getPersistentId().toString(), QClipboard::Selection);
        }
    } else if (selectedItem == setTopicAction) {
        editTopic();
    } else if (selectedItem == setNicknameAction) {
        setNickname();
    } else if (selectedItem == statusOnlineAction) {
        group->setGroupStatus(Status::Status::Online);
    } else if (selectedItem == statusAwayAction) {
        group->setGroupStatus(Status::Status::Away);
    } else if (selectedItem == statusBusyAction) {
        group->setGroupStatus(Status::Status::Busy);
    } else if (selectedItem == setPasswordAction) {
        setPassword();
    } else if (selectedItem == clearPasswordAction) {
        clearPassword();
    } else if (selectedItem == setPeerLimitAction) {
        setPeerLimit();
    } else if (selectedItem == setTopicLockAction) {
        group->setGroupTopicLock(setTopicLockAction->isChecked() ? GroupTopicLock::Enabled
                                                                 : GroupTopicLock::Disabled);
    } else if (selectedItem == voiceAllAction) {
        group->setGroupVoiceState(GroupVoiceState::All);
    } else if (selectedItem == voiceModeratorAction) {
        group->setGroupVoiceState(GroupVoiceState::Moderator);
    } else if (selectedItem == voiceFounderAction) {
        group->setGroupVoiceState(GroupVoiceState::Founder);
    } else if (selectedItem == privacyPublicAction) {
        group->setGroupPrivacyState(GroupPrivacyState::Public);
    } else if (selectedItem == privacyPrivateAction) {
        group->setGroupPrivacyState(GroupPrivacyState::Private);
    }
}

void GroupForm::setPassword()
{
    bool ok = false;
    const QString password = QInputDialog::getText(this, tr("Set group password"),
                                                   tr("Password:"), QLineEdit::Password, QString(),
                                                   &ok);
    if (ok) {
        group->setGroupPassword(password.toUtf8());
    }
}

void GroupForm::setNickname()
{
    bool ok = false;
    const QString nickname = QInputDialog::getText(this, tr("Set nickname"),
                                                   tr("Nickname:"), QLineEdit::Normal,
                                                   group->getGroupNickname(), &ok);
    if (ok) {
        group->setGroupNickname(nickname);
    }
}

void GroupForm::clearPassword()
{
    group->setGroupPassword({});
}

void GroupForm::setPeerLimit()
{
    bool ok = false;
    const int peerLimit = QInputDialog::getInt(this, tr("Set peer limit"), tr("Peer limit:"),
                                               group->getPeerLimit(), 0, 65535, 1, &ok);
    if (ok) {
        group->setGroupPeerLimit(static_cast<uint16_t>(peerLimit));
    }
}

void GroupForm::editTopic()
{
    if (!canSetTopic()) {
        return;
    }

    bool ok = false;
    const QString topic = QInputDialog::getMultiLineText(
        this, tr("Set group topic"), tr("Topic:"), group->getTopic(), &ok);
    if (ok) {
        core.changeGroupTopic(group->getId(), topic);
    }
}

bool GroupForm::canSetTopic() const
{
    const GroupRole selfRole = group->getPeerRole(core.getGroupSelfPk(group->getId()));
    if (selfRole == GroupRole::Observer) {
        return false;
    }

    if (group->getTopicLock() == GroupTopicLock::Disabled) {
        return true;
    }

    return selfRole == GroupRole::Founder || selfRole == GroupRole::Moderator;
}

void GroupForm::startPrivateMessage(const ToxPk& peerPk)
{
    privateMessageTarget = peerPk;
    updatePrivateMessageIndicator();
    privateMessageBar->show();
    msgEdit->setFocus();
}

void GroupForm::cancelPrivateMessage()
{
    privateMessageTarget = ToxPk{};
    privateMessageBar->hide();
}

void GroupForm::updatePrivateMessageIndicator()
{
    if (privateMessageTarget.isEmpty()) {
        privateMessageLabel->clear();
    } else {
        const QString peerName = group->getDisplayedName(privateMessageTarget);
        privateMessageLabel->setText(tr("Private message to: %1").arg(peerName));
    }
}

void GroupForm::onSendTriggered()
{
    auto msg = msgEdit->toPlainText();

    const bool isAction = msg.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive);
    if (isAction) {
        msg.remove(0, ChatForm::ACTION_PREFIX.length());
    }

    if (msg.isEmpty()) {
        return;
    }

    if (!privateMessageTarget.isEmpty()) {
        if (groupDispatcher == nullptr) {
            const auto curTime = QDateTime::currentDateTime();
            addSystemInfoMessage(curTime, SystemMessageType::messageSendFailed, {});
            cancelPrivateMessage();
            return;
        }

        const uint32_t peerId = group->getPeerId(privateMessageTarget);
        if (peerId == std::numeric_limits<uint32_t>::max()) {
            const auto curTime = QDateTime::currentDateTime();
            addSystemInfoMessage(curTime, SystemMessageType::messageSendFailed, {});
            cancelPrivateMessage();
            return;
        }

        msgEdit->setLastMessage(msg);
        msgEdit->clear();
        groupDispatcher->sendPrivateMessage(peerId, isAction, msg);
    } else {
        msgEdit->setLastMessage(msg);
        msgEdit->clear();
        messageDispatcher.sendMessage(isAction, msg);
    }
}