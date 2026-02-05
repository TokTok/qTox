/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "conferenceform.h"

#include "tabcompleter.h"

#include "src/chatlog/chatwidget.h"
#include "src/chatlog/content/text.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/friendlist.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include "src/widget/chatformheader.h"
#include "src/widget/conferencewidget.h"
#include "src/widget/flowlayout.h"
#include "src/widget/form/chatform.h"
#include "src/widget/style.h"
#include "src/widget/translator.h"

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QTimer>
#include <QToolButton>

namespace {
const auto LABEL_PEER_TYPE_OUR = QVariant(QStringLiteral("our"));
const auto LABEL_PEER_TYPE_MUTED = QVariant(QStringLiteral("muted"));
const auto LABEL_PEER_PLAYING_AUDIO = QVariant(QStringLiteral("true"));
const auto LABEL_PEER_NOT_PLAYING_AUDIO = QVariant(QStringLiteral("false"));
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

/**
 * @var QList<QLabel*> ConferenceForm::peerLabels
 * @brief Maps peernumbers to the QLabels in namesListLayout.
 *
 * @var QMap<int, QTimer*> ConferenceForm::peerAudioTimers
 * @brief Timeout = peer stopped sending audio.
 */

ConferenceForm::ConferenceForm(Core& core_, Conference* chatConference, IChatLog& chatLog_,
                               IMessageDispatcher& messageDispatcher_, Settings& settings_,
                               DocumentCache& documentCache_, SmileyPack& smileyPack_, Style& style_,
                               IMessageBoxManager& messageBoxManager, FriendList& friendList_,
                               ConferenceList& conferenceList_, ImageLoader& imageLoader)
    : GenericChatForm(core_, chatConference, chatLog_, messageDispatcher_, documentCache_, smileyPack_,
                      settings_, style_, messageBoxManager, friendList_, conferenceList_, imageLoader)
    , core{core_}
    , conference(chatConference)
    , inCall(false)
    , settings(settings_)
    , style{style_}
    , friendList{friendList_}
{
    nusersLabel = new QLabel();

    tabber = new TabCompleter(msgEdit, conference);

    fileButton->setEnabled(false);
    fileButton->setProperty("state", "");
    ChatFormHeader::Mode mode = ChatFormHeader::Mode::None;
    if (conference->isAvConference()) {
        mode = ChatFormHeader::Mode::Audio;
    }

    headWidget->setMode(mode);
    setName(conference->getName());

    nusersLabel->setFont(Style::getFont(Style::Font::Medium));
    nusersLabel->setObjectName("statusLabel");
    retranslateUi();

    const QSize& size = headWidget->getAvatarSize();
    headWidget->setAvatar(
        Style::scaleSvgImage(":/img/conference_dark.svg", size.width(), size.height()));

    msgEdit->setObjectName("conference");

    namesListLayout = new FlowLayout(0, 5, 0);
    headWidget->addWidget(nusersLabel);
    headWidget->addLayout(namesListLayout);
    headWidget->addStretch();

    // nameLabel->setMinimumHeight(12);
    nusersLabel->setMinimumHeight(12);

    connect(msgEdit, &ChatTextEdit::tabPressed, tabber, &TabCompleter::complete);
    connect(msgEdit, &ChatTextEdit::keyPressed, tabber, &TabCompleter::reset);
    connect(headWidget, &ChatFormHeader::callTriggered, this, &ConferenceForm::onCallClicked);
    connect(headWidget, &ChatFormHeader::micMuteToggle, this, &ConferenceForm::onMicMuteToggle);
    connect(headWidget, &ChatFormHeader::volMuteToggle, this, &ConferenceForm::onVolMuteToggle);
    connect(headWidget, &ChatFormHeader::nameChanged, chatConference, &Conference::setName);
    connect(conference, &Conference::titleChanged, this, &ConferenceForm::onTitleChanged);
    connect(conference, &Conference::userJoined, this, &ConferenceForm::onUserJoined);
    connect(conference, &Conference::userLeft, this, &ConferenceForm::onUserLeft);
    connect(conference, &Conference::peerNameChanged, this, &ConferenceForm::onPeerNameChanged);
    connect(conference, &Conference::numPeersChanged, this, &ConferenceForm::updateUserCount);
    settings.connectTo_blockListChanged(this, [this](const QStringList&) { updateUserNames(); });

    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::selfJoinedConference, {});
    }

    updateUserNames();
    setAcceptDrops(true);
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

ConferenceForm::~ConferenceForm()
{
    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::selfLeftConference, {});
    }
    Translator::unregister(this);
}

void ConferenceForm::onTitleChanged(const QString& author, const QString& title)
{
    if (author.isEmpty()) {
        return;
    }

    const QDateTime curTime = QDateTime::currentDateTime();
    addSystemInfoMessage(curTime, SystemMessageType::titleChanged, {author, title});
}

void ConferenceForm::onScreenshotClicked()
{
    // Unsupported
}

void ConferenceForm::onAttachClicked()
{
    // Unsupported
}

/**
 * @brief Updates user names' labels at the top of conference
 */
void ConferenceForm::updateUserNames()
{
    QLayoutItem* child;
    while ((child = namesListLayout->takeAt(0)) != nullptr) {
        child->widget()->hide();
        delete child->widget();
        delete child;
    }

    peerLabels.clear();
    const auto peers = conference->getPeerList();

    // no need to do anything without any peers
    if (peers.isEmpty()) {
        return;
    }

    /* we store the peer labels by their ToxPk, but the namelist layout
     * needs it in alphabetical order, so we first create and store the labels
     * and then sort them by their text and add them to the layout in that order */
    const auto selfPk = core.getSelfPublicKey();
    for (const auto& peerPk : peers.keys()) {
        const QString peerName = peers.value(peerPk);
        const QString editedName = editName(peerName);
        auto* const label = new QLabel(editedName + QLatin1String(", "));
        if (editedName != peerName) {
            label->setToolTip(peerName + " (" + peerPk.toString() + ")");
        } else if (peerName != peerPk.toString()) {
            label->setToolTip(peerPk.toString());
        } // else their name is just their Pk, no tooltip needed
        label->setTextFormat(Qt::PlainText);
        label->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(label, &QLabel::customContextMenuRequested, this,
                &ConferenceForm::onLabelContextMenuRequested);

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
        return a->text().toLower() < b->text().toLower();
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

void ConferenceForm::onUserJoined(const ToxPk& user, const QString& name)
{
    std::ignore = user;
    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::userJoinedConference,
                             {name});
    }
    updateUserNames();
}

void ConferenceForm::onUserLeft(const ToxPk& user, const QString& name)
{
    std::ignore = user;
    if (settings.getShowConferenceJoinLeaveMessages()) {
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::userLeftConference,
                             {name});
    }
    updateUserNames();
}

void ConferenceForm::onPeerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName)
{
    std::ignore = peer;
    addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::peerNameChanged,
                         {oldName, newName});
    updateUserNames();
}

void ConferenceForm::peerAudioPlaying(ToxPk peerPk)
{
    peerLabels[peerPk]->setProperty("playingAudio", LABEL_PEER_PLAYING_AUDIO);
    peerLabels[peerPk]->style()->unpolish(peerLabels[peerPk]);
    peerLabels[peerPk]->style()->polish(peerLabels[peerPk]);
    // TODO(sudden6): check if this can ever be false, cause [] default constructs
    if (peerAudioTimers[peerPk] == nullptr) {
        peerAudioTimers[peerPk] = new QTimer(this);
        peerAudioTimers[peerPk]->setSingleShot(true);
        connect(peerAudioTimers[peerPk], &QTimer::timeout, this, [this, peerPk] {
            auto it = peerLabels.find(peerPk);
            if (it != peerLabels.end()) {
                peerLabels[peerPk]->setProperty("playingAudio", LABEL_PEER_NOT_PLAYING_AUDIO);
                peerLabels[peerPk]->style()->unpolish(peerLabels[peerPk]);
                peerLabels[peerPk]->style()->polish(peerLabels[peerPk]);
            }
            delete peerAudioTimers[peerPk];
            peerAudioTimers[peerPk] = nullptr;
        });
    }

    peerLabels[peerPk]->setStyleSheet(style.getStylesheet(PEER_LABEL_STYLE_SHEET_PATH, settings));
    peerAudioTimers[peerPk]->start(500);
}

void ConferenceForm::dragEnterEvent(QDragEnterEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk toxPk{ev->mimeData()->data("toxPk")};
    Friend* frnd = friendList.findFriend(toxPk);
    if (frnd != nullptr)
        ev->acceptProposedAction();
}

void ConferenceForm::dropEvent(QDropEvent* ev)
{
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    const ToxPk toxPk{ev->mimeData()->data("toxPk")};
    Friend* frnd = friendList.findFriend(toxPk);
    if (frnd == nullptr)
        return;

    const int friendId = frnd->getId();
    const int conferenceId = conference->getId();
    if (Status::isOnline(frnd->getStatus())) {
        core.conferenceInviteFriend(friendId, conferenceId);
    }
}

void ConferenceForm::onMicMuteToggle()
{
    if (audioInputFlag) {
        CoreAV* av = core.getAv();
        const bool oldMuteState = av->isConferenceCallInputMuted(conference);
        const bool newMute = !oldMuteState;
        av->muteCallInput(conference, newMute);
        headWidget->updateMuteMicButton(inCall, newMute);
    }
}

void ConferenceForm::onVolMuteToggle()
{
    if (audioOutputFlag) {
        CoreAV* av = core.getAv();
        const bool oldMuteState = av->isConferenceCallOutputMuted(conference);
        const bool newMute = !oldMuteState;
        av->muteCallOutput(conference, newMute);
        headWidget->updateMuteVolButton(inCall, newMute);
    }
}

void ConferenceForm::onCallClicked()
{
    CoreAV* av = core.getAv();

    if (!inCall) {
        joinConferenceCall();
    } else {
        leaveConferenceCall();
    }

    headWidget->updateCallButtons(true, inCall);

    const bool inMute = av->isConferenceCallInputMuted(conference);
    headWidget->updateMuteMicButton(inCall, inMute);

    const bool outMute = av->isConferenceCallOutputMuted(conference);
    headWidget->updateMuteVolButton(inCall, outMute);
}

void ConferenceForm::keyPressEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && ((ev->modifiers() & Qt::ControlModifier) != 0u) && inCall) {
        onMicMuteToggle();
    }

    // TODO(iphydf): This does nothing. Also, this whole function is never called.
    // It should prevent tabbing out of the editor widget when tab-completing, but it doesn't.
    if (msgEdit->hasFocus())
        return;
}

void ConferenceForm::keyReleaseEvent(QKeyEvent* ev)
{
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && ((ev->modifiers() & Qt::ControlModifier) != 0u) && inCall) {
        onMicMuteToggle();
    }

    if (msgEdit->hasFocus())
        return;
}

/**
 * @brief Updates users' count label text
 */
void ConferenceForm::updateUserCount(int numPeers)
{
    nusersLabel->setText(tr("%n user(s) in chat", "Number of users in chat", numPeers));
    headWidget->updateCallButtons(true, inCall);
}

void ConferenceForm::retranslateUi()
{
    updateUserCount(conference->getPeersCount());
}

void ConferenceForm::onLabelContextMenuRequested(const QPoint& localPos)
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
    const ToxPk selfPk = core.getSelfPublicKey();
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

    const QAction* selectedItem = contextMenu->exec(pos);
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
    }
}

void ConferenceForm::joinConferenceCall()
{
    CoreAV* av = core.getAv();
    av->joinConferenceCall(*conference);
    audioInputFlag = true;
    audioOutputFlag = true;
    inCall = true;
    emit startConferenceCallNotification(conference->getId());
}

void ConferenceForm::leaveConferenceCall()
{
    CoreAV* av = core.getAv();
    av->leaveConferenceCall(conference->getId());
    audioInputFlag = false;
    audioOutputFlag = false;
    inCall = false;
    emit endConferenceCallNotification(conference->getId());
}
