/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "chatform.h"

#include "src/chatlog/chatmessage.h"
#include "src/chatlog/chatwidget.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/chatlog/content/text.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/corefile.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/platform/screenshot.h"
#include "src/video/netcamview.h"
#include "src/widget/chatformheader.h"
#include "src/widget/contentdialogmanager.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/widget/imagepreviewwidget.h"
#include "src/widget/searchform.h"
#include "src/widget/style.h"
#include "src/widget/tool/abstractscreenshotgrabber.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/tool/screenshotgrabber.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

#include <QApplication>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QStringBuilder>

#include <cassert>
#include <memory>

/**
 * @brief ChatForm::incomingNotification Notify that we are called by someone.
 * @param friendId Friend that is calling us.
 *
 * @brief ChatForm::outgoingNotification Notify that we are calling someone.
 *
 * @brief stopNotification Tell others to stop notification of a call.
 */
namespace {
constexpr int CHAT_WIDGET_MIN_HEIGHT = 50;
constexpr int SCREENSHOT_GRABBER_OPENING_DELAY = 500;
constexpr int TYPING_NOTIFICATION_DURATION = 3000;
} // namespace

const QString ChatForm::ACTION_PREFIX = QStringLiteral("/me ");

namespace {
QString secondsToDHMS(quint32 duration)
{
    const QString res;
    const QString cD = ChatForm::tr("Call duration: ");
    const quint32 seconds = duration % 60;
    duration /= 60;
    const quint32 minutes = duration % 60;
    duration /= 60;
    const quint32 hours = duration % 24;
    const quint32 days = duration / 24;

    // I assume no one will ever have call longer than a month
    if (days != 0u) {
        return cD + QString::asprintf("%dd%02dh %02dm %02ds", days, hours, minutes, seconds);
    }

    if (hours != 0u) {
        return cD + QString::asprintf("%02dh %02dm %02ds", hours, minutes, seconds);
    }

    if (minutes != 0u) {
        return cD + QString::asprintf("%02dm %02ds", minutes, seconds);
    }

    return cD + QString::asprintf("%02ds", seconds);
}
} // namespace

ChatForm::ChatForm(Profile& profile_, Friend* chatFriend, IChatLog& chatLog_,
                   IMessageDispatcher& messageDispatcher_, DocumentCache& documentCache_,
                   SmileyPack& smileyPack_, CameraSource& cameraSource_, Settings& settings_,
                   Style& style_, IMessageBoxManager& messageBoxManager,
                   ContentDialogManager& contentDialogManager_, FriendList& friendList_,
                   ConferenceList& conferenceList_, QWidget* parent_)
    : GenericChatForm(profile_.getCore(), chatFriend, chatLog_, messageDispatcher_, documentCache_,
                      smileyPack_, settings_, style_, messageBoxManager, friendList_,
                      conferenceList_, parent_)
    , core{profile_.getCore()}
    , f(chatFriend)
    , isTyping{false}
    , lastCallIsVideo{false}
    , cameraSource{cameraSource_}
    , settings{settings_}
    , style{style_}
    , contentDialogManager{contentDialogManager_}
    , profile{profile_}
{
    setName(f->getDisplayedName());

    headWidget->setAvatar(QPixmap(":/img/contact_dark.svg"));

    statusMessageLabel = new CroppingLabel();
    statusMessageLabel->setObjectName("statusLabel");
    statusMessageLabel->setFont(Style::getFont(Style::Font::Medium));
    statusMessageLabel->setMinimumHeight(Style::getFont(Style::Font::Medium).pixelSize());
    statusMessageLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    typingTimer.setSingleShot(true);

    callDurationTimer = nullptr;

    chatWidget->setMinimumHeight(CHAT_WIDGET_MIN_HEIGHT);

    callDuration = new QLabel();
    headWidget->addWidget(statusMessageLabel);
    headWidget->addStretch();
    headWidget->addWidget(callDuration, 1, Qt::AlignCenter);
    callDuration->hide();

    imagePreview = new ImagePreviewButton(this);
    imagePreview->setFixedSize(100, 100);
    imagePreview->setFlat(true);
    imagePreview->setStyleSheet("QPushButton { border: 0px }");
    imagePreview->hide();

    auto cancelIcon = QIcon(style.getImagePath("rejectCall/rejectCall.svg", settings));
    auto* cancelButton = new QPushButton(imagePreview);
    cancelButton->setFixedSize(20, 20);
    cancelButton->move(QPoint(80, 0));
    cancelButton->setIcon(cancelIcon);
    cancelButton->setFlat(true);

    connect(cancelButton, &QPushButton::pressed, this, &ChatForm::cancelImagePreview);

    contentLayout->insertWidget(3, imagePreview);

    copyStatusAction = statusMessageMenu.addAction(QString(), this, &ChatForm::onCopyStatusMessage);

    const CoreFile* coreFile = core.getCoreFile();
    connect(&profile, &Profile::friendAvatarChanged, this, &ChatForm::onAvatarChanged);
    connect(coreFile, &CoreFile::fileReceiveRequested, this, &ChatForm::updateFriendActivityForFile);
    connect(coreFile, &CoreFile::fileSendStarted, this, &ChatForm::updateFriendActivityForFile);
    connect(&core, &Core::friendTypingChanged, this, &ChatForm::onFriendTypingChanged);
    connect(coreFile, &CoreFile::fileNameChanged, this, &ChatForm::onFileNameChanged);

    connect(chatFriend, &Friend::statusChanged, this, &ChatForm::onFriendStatusChanged);

    const CoreAV* av = core.getAv();
    connect(av, &CoreAV::avInvite, this, &ChatForm::onAvInvite);
    connect(av, &CoreAV::avStart, this, &ChatForm::onAvStart);
    connect(av, &CoreAV::avEnd, this, &ChatForm::onAvEnd);

    connect(headWidget, &ChatFormHeader::callTriggered, this, &ChatForm::onCallTriggered);
    connect(headWidget, &ChatFormHeader::videoCallTriggered, this, &ChatForm::onVideoCallTriggered);
    connect(headWidget, &ChatFormHeader::micMuteToggle, this, &ChatForm::onMicMuteToggle);
    connect(headWidget, &ChatFormHeader::volMuteToggle, this, &ChatForm::onVolMuteToggle);
    connect(sendButton, &QPushButton::pressed, this, &ChatForm::callUpdateFriendActivity);
    connect(sendButton, &QPushButton::pressed, this, &ChatForm::sendImageFromPreview);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::callUpdateFriendActivity);
    connect(msgEdit, &ChatTextEdit::textChanged, this, &ChatForm::onTextEditChanged);
    connect(msgEdit, &ChatTextEdit::pasteImage, this, &ChatForm::previewImage);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::sendImageFromPreview);
    connect(msgEdit, &ChatTextEdit::escapePressed, this, &ChatForm::cancelImagePreview);
    connect(statusMessageLabel, &CroppingLabel::customContextMenuRequested, this,
            [&](const QPoint& pos) {
                if (!statusMessageLabel->text().isEmpty()) {
                    auto* sender_ = static_cast<QWidget*>(sender());
                    statusMessageMenu.exec(sender_->mapToGlobal(pos));
                }
            });

    connect(&typingTimer, &QTimer::timeout, this, [&] {
        core.sendTyping(f->getId(), false);
        isTyping = false;
    });

    // reflect name changes in the header
    connect(headWidget, &ChatFormHeader::nameChanged, this,
            [this](const QString& newName) { f->setAlias(newName); });
    connect(headWidget, &ChatFormHeader::callAccepted, this,
            [this] { onAnswerCallTriggered(lastCallIsVideo); });
    connect(headWidget, &ChatFormHeader::callRejected, this, &ChatForm::onRejectCallTriggered);

    connect(bodySplitter, &QSplitter::splitterMoved, this, &ChatForm::onSplitterMoved);

    updateCallButtons();

    setAcceptDrops(true);
    retranslateUi();
    Translator::registerHandler([this] { retranslateUi(); }, this);
}

ChatForm::~ChatForm()
{
    Translator::unregister(this);
}

void ChatForm::setStatusMessage(const QString& newMessage)
{
    statusMessageLabel->setText(newMessage);
    // for long messages
    statusMessageLabel->setToolTip(Qt::convertFromPlainText(newMessage, Qt::WhiteSpaceNormal));
}

void ChatForm::callUpdateFriendActivity()
{
    emit updateFriendActivity(*f);
}

void ChatForm::updateFriendActivityForFile(const ToxFile& file)
{
    if (file.friendId != f->getId()) {
        return;
    }
    emit updateFriendActivity(*f);
}

void ChatForm::onFileNameChanged(const ToxPk& friendPk)
{
    if (friendPk != f->getPublicKey()) {
        return;
    }

    QMessageBox::warning(this, tr("Filename contained illegal characters"),
                         tr("Illegal characters have been changed to _\n"
                            "so you can save the file on Windows."));
}

void ChatForm::onTextEditChanged()
{
    if (!settings.getTypingNotification()) {
        if (isTyping) {
            isTyping = false;
            core.sendTyping(f->getId(), false);
        }

        return;
    }
    const bool isTypingNow = !msgEdit->toPlainText().isEmpty();
    if (isTyping != isTypingNow) {
        core.sendTyping(f->getId(), isTypingNow);
        if (isTypingNow) {
            typingTimer.start(TYPING_NOTIFICATION_DURATION);
        }

        isTyping = isTypingNow;
    }
}

void ChatForm::onAttachClicked()
{
    const QStringList paths = QFileDialog::getOpenFileNames(Q_NULLPTR, tr("Send a file"),
                                                            QDir::homePath(), QString(), nullptr);

    if (paths.isEmpty()) {
        return;
    }

    for (const QString& path : paths) {
        QFile file(path);
        const QString fileName = QFileInfo(path).fileName();
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Unable to open"),
                                 tr("qTox wasn't able to open %1").arg(fileName));
            continue;
        }

        file.close();
        if (file.isSequential()) {
            QMessageBox::critical(this, tr("Bad idea"),
                                  tr("You're trying to send a sequential file, "
                                     "which is not going to work!"));
            continue;
        }

        const qint64 filesize = file.size();
        core.getCoreFile()->sendFile(f->getId(), fileName, path, filesize);
    }
}

void ChatForm::onAvInvite(uint32_t friendId, bool video)
{
    if (friendId != f->getId()) {
        return;
    }

    const QString displayedName = f->getDisplayedName();

    SystemMessage systemMessage;
    systemMessage.messageType = SystemMessageType::incomingCall;
    systemMessage.args = {displayedName};
    chatLog.addSystemMessage(systemMessage);

    auto testedFlag = video ? Settings::AutoAcceptCall::Video : Settings::AutoAcceptCall::Audio;
    // AutoAcceptCall is set for this friend
    if (settings.getAutoAcceptCall(f->getPublicKey()).testFlag(testedFlag)) {
        qDebug() << "automatic call answer";
        CoreAV* coreAv = core.getAv();
        QMetaObject::invokeMethod(coreAv, "answerCall", Qt::QueuedConnection,
                                  Q_ARG(uint32_t, friendId), Q_ARG(bool, video));
        onAvStart(friendId, video);
    } else {
        headWidget->createCallConfirm(video);
        headWidget->showCallConfirm();
        lastCallIsVideo = video;
        emit incomingNotification(friendId);
    }
}

void ChatForm::onAvStart(uint32_t friendId, bool video)
{
    if (friendId != f->getId()) {
        return;
    }

    if (video) {
        showNetcam();
    } else {
        hideNetcam();
    }

    emit stopNotification();
    updateCallButtons();
    startCounter();
}

void ChatForm::onAvEnd(uint32_t friendId, bool error)
{
    if (friendId != f->getId()) {
        return;
    }

    headWidget->removeCallConfirm();
    // Fixes an macOS bug with ending a call while in full screen
    if (netcam && netcam->isFullScreen()) {
        netcam->showNormal();
    }

    emit stopNotification();
    emit endCallNotification();
    updateCallButtons();
    stopCounter(error);
    hideNetcam();
}

void ChatForm::showOutgoingCall(bool video)
{
    headWidget->showOutgoingCall(video);
    addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::outgoingCall,
                         {f->getDisplayedName()});
    emit outgoingNotification();
    emit updateFriendActivity(*f);
}

void ChatForm::onAnswerCallTriggered(bool video)
{
    headWidget->removeCallConfirm();
    const uint32_t friendId = f->getId();
    emit stopNotification();
    emit acceptCall(friendId);

    updateCallButtons();
    CoreAV* av = core.getAv();
    if (!av->answerCall(friendId, video)) {
        updateCallButtons();
        stopCounter();
        hideNetcam();
        return;
    }

    onAvStart(friendId, av->isCallVideoEnabled(f));
}

void ChatForm::onRejectCallTriggered()
{
    headWidget->removeCallConfirm();
    emit rejectCall(f->getId());
}

void ChatForm::onCallTriggered()
{
    CoreAV* av = core.getAv();
    const uint32_t friendId = f->getId();
    if (av->isCallStarted(f)) {
        av->cancelCall(friendId);
    } else if (av->startCall(friendId, false)) {
        showOutgoingCall(false);
    }
}

void ChatForm::onVideoCallTriggered()
{
    CoreAV* av = core.getAv();
    const uint32_t friendId = f->getId();
    if (av->isCallStarted(f)) {
        // TODO: We want to activate video on the active call.
        if (av->isCallVideoEnabled(f)) {
            av->cancelCall(friendId);
        }
    } else if (av->startCall(friendId, true)) {
        showOutgoingCall(true);
    }
}

void ChatForm::updateCallButtons()
{
    CoreAV* av = core.getAv();
    const bool audio = av->isCallActive(f);
    const bool video = av->isCallVideoEnabled(f);
    const bool online = Status::isOnline(f->getStatus());
    headWidget->updateCallButtons(online, audio, video);
    updateMuteMicButton();
    updateMuteVolButton();
}

void ChatForm::onMicMuteToggle()
{
    CoreAV* av = core.getAv();
    av->toggleMuteCallInput(f);
    updateMuteMicButton();
}

void ChatForm::onVolMuteToggle()
{
    CoreAV* av = core.getAv();
    av->toggleMuteCallOutput(f);
    updateMuteVolButton();
}

void ChatForm::onFriendStatusChanged(const ToxPk& friendPk, Status::Status status)
{
    // Disable call buttons if friend is offline
    assert(friendPk == f->getPublicKey());
    std::ignore = friendPk;

    if (!Status::isOnline(f->getStatus())) {
        // Hide the "is typing" message when a friend goes offline
        setFriendTyping(false);
    }

    updateCallButtons();

    if (settings.getStatusChangeNotificationEnabled()) {
        const QString fStatus = Status::getTitle(status);
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::peerStateChange,
                             {f->getDisplayedName(), fStatus});
    }
}

void ChatForm::onFriendTypingChanged(quint32 friendId, bool isTyping_)
{
    if (friendId == f->getId()) {
        setFriendTyping(isTyping_);
    }
}

void ChatForm::onFriendNameChanged(const QString& name)
{
    if (sender() == f) {
        setName(name);
    }
}

void ChatForm::onStatusMessage(const QString& message)
{
    if (sender() == f) {
        setStatusMessage(message);
    }
}

void ChatForm::onAvatarChanged(const ToxPk& friendPk, const QPixmap& pic)
{
    if (friendPk != f->getPublicKey()) {
        return;
    }

    headWidget->setAvatar(pic);
}

std::unique_ptr<NetCamView> ChatForm::createNetcam()
{
    qDebug() << "creating netcam";
    const uint32_t friendId = f->getId();
    std::unique_ptr<NetCamView> view =
        std::make_unique<NetCamView>(f->getPublicKey(), cameraSource, settings, style, profile, this);
    CoreAV* av = core.getAv();
    VideoSource* source = av->getVideoSourceFromCall(friendId);
    view->show(source, f->getDisplayedName());
    connect(view.get(), &NetCamView::videoCallEnd, this, &ChatForm::onVideoCallTriggered);
    connect(view.get(), &NetCamView::volMuteToggle, this, &ChatForm::onVolMuteToggle);
    connect(view.get(), &NetCamView::micMuteToggle, this, &ChatForm::onMicMuteToggle);
    return view;
}

void ChatForm::dragEnterEvent(QDragEnterEvent* ev)
{
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}

void ChatForm::dropEvent(QDropEvent* ev)
{
    if (!ev->mimeData()->hasUrls()) {
        return;
    }

    for (const QUrl& url : ev->mimeData()->urls()) {
        QFileInfo info(url.path());
        QFile file(info.absoluteFilePath());

        const QString urlString = url.toString();
        if (url.isValid() && !url.isLocalFile()
            && urlString.length() < static_cast<int>(tox_max_message_length())) {
            messageDispatcher.sendMessage(false, urlString);

            continue;
        }

        const QString fileName = info.fileName();
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            info.setFile(url.toLocalFile());
            file.setFileName(info.absoluteFilePath());
            if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, tr("Unable to open"),
                                     tr("qTox wasn't able to open %1").arg(fileName));
                continue;
            }
        }

        file.close();
        if (file.isSequential()) {
            QMessageBox::critical(nullptr, tr("Bad idea"),
                                  tr("You're trying to send a sequential file, "
                                     "which is not going to work!"));
            continue;
        }

        if (info.exists()) {
            core.getCoreFile()->sendFile(f->getId(), fileName, info.absoluteFilePath(), info.size());
        }
    }
}

void ChatForm::onScreenshotClicked()
{
    doScreenshot();
    // Give the window manager a moment to open the full-screen grabber window
    QTimer::singleShot(SCREENSHOT_GRABBER_OPENING_DELAY, this, &ChatForm::hideFileMenu);
}

void ChatForm::doScreenshot()
{
    // Note: grabber is self-managed and will destroy itself when done.
    AbstractScreenshotGrabber* grabber = Platform::createScreenshotGrabber(this);
    if (grabber == nullptr) {
        grabber = new ScreenshotGrabber(this);
    }
    connect(grabber, &AbstractScreenshotGrabber::screenshotTaken, this, &ChatForm::previewImage);
    grabber->showGrabber();
}

void ChatForm::previewImage(const QPixmap& pixmap)
{
    imagePreviewSource = pixmap;
    imagePreview->setIconFromPixmap(pixmap);
    imagePreview->show();
}

void ChatForm::cancelImagePreview()
{
    imagePreviewSource = QPixmap();
    imagePreview->hide();
}

void ChatForm::sendImageFromPreview()
{
    if (!imagePreview->isVisible()) {
        return;
    }

    QDir(settings.getPaths().getAppDataDirPath()).mkpath("images");

    // use ~ISO 8601 for screenshot timestamp, considering FS limitations
    // https://en.wikipedia.org/wiki/ISO_8601
    // Windows has to be supported, thus filename can't have `:` in it :/
    // Format should be: `qTox_Screenshot_yyyy-MM-dd HH-mm-ss.zzz.png`
    const QString filepath =
        QString("%1images%2qTox_Image_%3.png")
            .arg(settings.getPaths().getAppDataDirPath())
            .arg(QDir::separator())
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss.zzz"));
    QFile file(filepath);

    if (file.open(QFile::ReadWrite)) {
        imagePreviewSource.save(&file, "PNG");
        const qint64 filesize = file.size();
        imagePreview->hide();
        imagePreviewSource = QPixmap();
        file.close();
        const QFileInfo fi(file);
        CoreFile* coreFile = core.getCoreFile();
        coreFile->sendFile(f->getId(), fi.fileName(), fi.filePath(), filesize);
    } else {
        QMessageBox::warning(this,
                             tr("Failed to open temporary file", "Temporary file for screenshot"),
                             tr("qTox wasn't able to save the screenshot"));
    }
}

void ChatForm::onCopyStatusMessage()
{
    // make sure to copy not truncated text directly from the friend
    const QString text = f->getStatusMessage();
    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard != nullptr) {
        clipboard->setText(text, QClipboard::Clipboard);
    }
}

void ChatForm::updateMuteMicButton()
{
    const CoreAV* av = core.getAv();
    const bool active = av->isCallActive(f);
    const bool inputMuted = av->isCallInputMuted(f);
    headWidget->updateMuteMicButton(active, inputMuted);
    if (netcam) {
        netcam->updateMuteMicButton(inputMuted);
    }
}

void ChatForm::updateMuteVolButton()
{
    const CoreAV* av = core.getAv();
    const bool active = av->isCallActive(f);
    const bool outputMuted = av->isCallOutputMuted(f);
    headWidget->updateMuteVolButton(active, outputMuted);
    if (netcam) {
        netcam->updateMuteVolButton(outputMuted);
    }
}

void ChatForm::startCounter()
{
    if (callDurationTimer != nullptr) {
        return;
    }
    callDurationTimer = new QTimer();
    connect(callDurationTimer, &QTimer::timeout, this, &ChatForm::onUpdateTime);
    callDurationTimer->start(1000);
    timeElapsed.start();
    callDuration->show();
}

void ChatForm::stopCounter(bool error)
{
    if (callDurationTimer == nullptr) {
        return;
    }
    const QString dhms = secondsToDHMS(timeElapsed.elapsed() / 1000);
    const QString name = f->getDisplayedName();
    auto messageType = error ? SystemMessageType::unexpectedCallEnd : SystemMessageType::callEnd;
    // TODO: add notification once notifications are implemented

    addSystemInfoMessage(QDateTime::currentDateTime(), messageType, {name, dhms});
    callDurationTimer->stop();
    callDuration->setText("");
    callDuration->hide();

    delete callDurationTimer;
    callDurationTimer = nullptr;
}

void ChatForm::onUpdateTime()
{
    callDuration->setText(secondsToDHMS(timeElapsed.elapsed() / 1000));
}

void ChatForm::setFriendTyping(bool isTyping_)
{
    chatWidget->setTypingNotificationVisible(isTyping_);
    const QString name = f->getDisplayedName();
    chatWidget->setTypingNotificationName(name);
}

void ChatForm::show(ContentLayout* contentLayout_)
{
    GenericChatForm::show(contentLayout_);
}

void ChatForm::reloadTheme()
{
    GenericChatForm::reloadTheme();
}

void ChatForm::showEvent(QShowEvent* event)
{
    GenericChatForm::showEvent(event);
}

void ChatForm::hideEvent(QHideEvent* event)
{
    GenericChatForm::hideEvent(event);
}

void ChatForm::retranslateUi()
{
    copyStatusAction->setText(tr("Copy"));

    updateMuteMicButton();
    updateMuteVolButton();

    if (netcam) {
        netcam->setShowMessages(chatWidget->isVisible());
    }
}

void ChatForm::showNetcam()
{
    if (!netcam) {
        netcam = createNetcam();
    }

    connect(netcam.get(), &NetCamView::showMessageClicked, this, &ChatForm::onShowMessagesClicked);

    bodySplitter->insertWidget(0, netcam.get());
    bodySplitter->setCollapsible(0, false);

    const QSize minSize = netcam->getSurfaceMinSize();
    ContentDialog* current = contentDialogManager.current();
    if (current != nullptr) {
        current->onVideoShow(minSize);
    }
}

void ChatForm::hideNetcam()
{
    if (!netcam) {
        return;
    }

    ContentDialog* current = contentDialogManager.current();
    if (current != nullptr) {
        current->onVideoHide();
    }

    netcam->close();
    netcam->hide();
    netcam.reset();
}

void ChatForm::onSplitterMoved(int pos, int index)
{
    std::ignore = pos;
    std::ignore = index;
    if (netcam) {
        netcam->setShowMessages(bodySplitter->sizes()[1] == 0);
    }
}

void ChatForm::onShowMessagesClicked()
{
    if (netcam) {
        if (bodySplitter->sizes()[1] == 0) {
            bodySplitter->setSizes({1, 1});
        } else {
            bodySplitter->setSizes({1, 0});
        }

        onSplitterMoved(0, 0);
    }
}
