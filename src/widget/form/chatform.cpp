/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
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
#include "src/model/friendchatstate.h"
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

namespace {
constexpr int CHAT_WIDGET_MIN_HEIGHT = 50;
constexpr int SCREENSHOT_GRABBER_OPENING_DELAY = 500;
} // namespace

const QString ChatForm::ACTION_PREFIX = QStringLiteral("/me ");

ChatForm::ChatForm(Profile& profile_, Friend* chatFriend, FriendChatState& chatState_,
                   IChatLog& chatLog_, IMessageDispatcher& messageDispatcher_,
                   DocumentCache& documentCache_, SmileyPack& smileyPack_,
                   CameraSource& cameraSource_, Settings& settings_, Style& style_,
                   IMessageBoxManager& messageBoxManager, ContentDialogManager& contentDialogManager_,
                   FriendList& friendList_, ConferenceList& conferenceList_, QWidget* parent_)
    : GenericChatForm(profile_.getCore(), chatFriend, chatLog_, messageDispatcher_, documentCache_,
                      smileyPack_, settings_, style_, messageBoxManager, friendList_,
                      conferenceList_, parent_)
    , core{profile_.getCore()}
    , f(chatFriend)
    , chatState(chatState_)
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
    connect(coreFile, &CoreFile::fileNameChanged, this, &ChatForm::onFileNameChanged);

    // Connect header buttons to FriendChatState
    connect(headWidget, &ChatFormHeader::callTriggered, this, [this] { chatState.startCall(false); });
    connect(headWidget, &ChatFormHeader::videoCallTriggered, this,
            [this] { chatState.startCall(true); });
    connect(headWidget, &ChatFormHeader::micMuteToggle, this, [this] { chatState.toggleMicMute(); });
    connect(headWidget, &ChatFormHeader::volMuteToggle, this, [this] { chatState.toggleVolMute(); });
    connect(sendButton, &QPushButton::pressed, this, &ChatForm::callUpdateFriendActivity);
    connect(sendButton, &QPushButton::pressed, this, &ChatForm::sendImageFromPreview);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::callUpdateFriendActivity);
    connect(msgEdit, &ChatTextEdit::textChanged, this,
            [this] { chatState.onTextChanged(!msgEdit->toPlainText().isEmpty()); });
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

    // reflect name changes in the header
    connect(headWidget, &ChatFormHeader::nameChanged, this,
            [this](const QString& newName) { f->setAlias(newName); });
    connect(headWidget, &ChatFormHeader::callAccepted, this,
            [this] { chatState.answerCall(chatState.lastCallIsVideo()); });
    connect(headWidget, &ChatFormHeader::callRejected, this, [this] { chatState.rejectCall(); });

    connect(bodySplitter, &QSplitter::splitterMoved, this, &ChatForm::onSplitterMoved);

    // Connect FriendChatState signals to UI updates
    connect(&chatState, &FriendChatState::callStarted, this, [this](bool video) {
        if (video) {
            showNetcam();
        } else {
            hideNetcam();
        }
    });

    connect(&chatState, &FriendChatState::callEnded, this, [this](bool error) {
        std::ignore = error;
        headWidget->removeCallConfirm();
        // Fixes a macOS bug with ending a call while in full screen
        if (netcam && netcam->isFullScreen()) {
            netcam->showNormal();
        }
        hideNetcam();
    });

    connect(&chatState, &FriendChatState::callRejected, this,
            [this] { headWidget->removeCallConfirm(); });

    connect(&chatState, &FriendChatState::incomingCall, this, [this](bool video, bool autoAccepted) {
        if (autoAccepted) {
            // Call was auto-accepted, UI update handled by callStarted
            return;
        }
        headWidget->createCallConfirm(video);
        headWidget->showCallConfirm();
    });

    connect(&chatState, &FriendChatState::outgoingCall, this, [this](bool video) {
        headWidget->showOutgoingCall(video);
        emit updateFriendActivity(*f);
    });

    connect(&chatState, &FriendChatState::callButtonStateChanged, this,
            [this](bool online, bool audio, bool video) {
                headWidget->updateCallButtons(online, audio, video);
            });

    connect(&chatState, &FriendChatState::micMuteChanged, this, [this](bool active, bool muted) {
        headWidget->updateMuteMicButton(active, muted);
        if (netcam) {
            netcam->updateMuteMicButton(muted);
        }
    });

    connect(&chatState, &FriendChatState::volMuteChanged, this, [this](bool active, bool muted) {
        headWidget->updateMuteVolButton(active, muted);
        if (netcam) {
            netcam->updateMuteVolButton(muted);
        }
    });

    connect(&chatState, &FriendChatState::callDurationUpdated, this, [this](const QString& text) {
        if (text.isEmpty()) {
            callDuration->setText("");
            callDuration->hide();
        } else {
            callDuration->setText(text);
            callDuration->show();
        }
    });

    connect(&chatState, &FriendChatState::friendTypingChanged, this, &ChatForm::setFriendTyping);

    connect(&chatState, &FriendChatState::friendStatusMessage, this,
            [this](QDateTime timestamp, SystemMessageType type, SystemMessage::Args args) {
                addSystemInfoMessage(timestamp, type, std::move(args));
            });

    connect(&chatState, &FriendChatState::acceptCallRequested, this, [this](uint32_t friendId) {
        headWidget->removeCallConfirm();
        std::ignore = friendId;
    });

    // Initial call button state
    chatState.updateCallButtons();

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
    connect(view.get(), &NetCamView::videoCallEnd, this, [this] { chatState.startCall(true); });
    connect(view.get(), &NetCamView::volMuteToggle, this, [this] { chatState.toggleVolMute(); });
    connect(view.get(), &NetCamView::micMuteToggle, this, [this] { chatState.toggleMicMute(); });
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
