/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "genericchatform.h"

#include "src/chatlog/chatwidget.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/chatlog/content/timestamp.h"
#include "src/conferencelist.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/chatformheader.h"
#include "src/widget/contentdialog.h"
#include "src/widget/contentlayout.h"
#include "src/widget/emoticonswidget.h"
#include "src/widget/form/chatform.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/widget/searchform.h"
#include "src/widget/style.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/flyoutoverlaywidget.h"
#include "src/widget/translator.h"
#include "src/widget/widget.h"

#include <QClipboard>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QtGlobal>

#ifdef SPELL_CHECKING
#include <sonnet/spellcheckdecorator.h>
#endif

/**
 * @class GenericChatForm
 * @brief Parent class for all chat forms. It's provide the minimum required UI
 * elements and methods to work with chat messages.
 */

namespace {
const QSize FILE_FLYOUT_SIZE{24, 24};
const short FOOT_BUTTONS_SPACING = 2;
const short MESSAGE_EDIT_HEIGHT = 50;
const short MAIN_FOOT_LAYOUT_SPACING = 5;
const QString FONT_STYLE[]{"normal", "italic", "oblique"};

/**
 * @brief Creates CSS style string for needed class with specified font
 * @param font Font that needs to be represented for a class
 * @param name Class name
 * @return Style string
 */
QString fontToCss(const QFont& font, const QString& name)
{
    const QString result{"%1{"
                         "font-family: \"%2\"; "
                         "font-size: %3px; "
                         "font-style: \"%4\"; "
                         "font-weight: normal;}"};
    return result.arg(name).arg(font.family()).arg(font.pixelSize()).arg(FONT_STYLE[font.style()]);
}
} // namespace

/**
 * @brief Searches for name (possibly alias) of someone with specified public key among all of your
 * friends or conferences you are participated
 * @param pk Searched public key
 * @return Name or alias of someone with such public key, or public key string representation if no
 * one was found
 */
QString GenericChatForm::resolveToxPk(const ToxPk& pk)
{
    Friend* f = friendList.findFriend(pk);
    if (f != nullptr) {
        return f->getDisplayedName();
    }

    for (Conference* it : conferenceList.getAllConferences()) {
        QString res = it->resolveToxPk(pk);
        if (!res.isEmpty()) {
            return res;
        }
    }

    return pk.toString();
}

namespace {
const QString STYLE_PATH = QStringLiteral("chatForm/buttons.qss");
}

namespace {

template <class T, class Fun>
QPushButton* createButton(const QString& name, T* self, Fun onClickSlot, Settings& settings, Style& style)
{
    auto* btn = new QPushButton();
    // Fix for incorrect layouts on macOS as per
    // https://bugreports.qt-project.org/browse/QTBUG-14591
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", "green");
    btn->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    QObject::connect(btn, &QPushButton::clicked, self, onClickSlot);
    return btn;
}

} // namespace

GenericChatForm::GenericChatForm(const Core& core_, const Chat* chat, IChatLog& chatLog_,
                                 IMessageDispatcher& messageDispatcher_, DocumentCache& documentCache,
                                 SmileyPack& smileyPack_, Settings& settings_, Style& style_,
                                 IMessageBoxManager& messageBoxManager, FriendList& friendList_,
                                 ConferenceList& conferenceList_, QWidget* parent_)
    : QWidget(parent_, Qt::Window)
    , core{core_}
    , audioInputFlag(false)
    , audioOutputFlag(false)
    , chatLog(chatLog_)
    , messageDispatcher(messageDispatcher_)
    , smileyPack{smileyPack_}
    , settings{settings_}
    , style{style_}
    , friendList{friendList_}
    , conferenceList{conferenceList_}
{
    curRow = 0;
    headWidget = new ChatFormHeader(settings, style);
    searchForm = new SearchForm(settings, style);
    dateInfo = new QLabel(this);
    chatWidget = new ChatWidget(chatLog_, core, documentCache, smileyPack, settings, style,
                                messageBoxManager, this);
    searchForm->hide();
    dateInfo->setAlignment(Qt::AlignHCenter);

    // settings
    connect(&settings, &Settings::emojiFontPointSizeChanged, chatWidget, &ChatWidget::forceRelayout);
    connect(&settings, &Settings::chatMessageFontChanged, this,
            &GenericChatForm::onChatMessageFontChanged);

    msgEdit = new ChatTextEdit(this);
#ifdef SPELL_CHECKING
    if (settings.getSpellCheckingEnabled()) {
        decorator = new Sonnet::SpellCheckDecorator(msgEdit);
    }
#endif

    sendButton = createButton("sendButton", this, &GenericChatForm::onSendTriggered, settings, style);
    emoteButton =
        createButton("emoteButton", this, &GenericChatForm::onEmoteButtonClicked, settings, style);

    fileButton = createButton("fileButton", this, &GenericChatForm::onAttachClicked, settings, style);
    screenshotButton = createButton("screenshotButton", this, &GenericChatForm::onScreenshotClicked,
                                    settings, style);

    // TODO: Make updateCallButtons (see ChatForm) abstract
    //       and call here to set tooltips.

    fileFlyout = new FlyoutOverlayWidget;
    auto* fileLayout = new QHBoxLayout(fileFlyout);
    fileLayout->addWidget(screenshotButton);
    fileLayout->setContentsMargins(0, 0, 0, 0);
    fileLayout->setSpacing(0);
    fileLayout->setContentsMargins(0, 0, 0, 0);

    msgEdit->setFixedHeight(MESSAGE_EDIT_HEIGHT);
    msgEdit->setFrameStyle(QFrame::NoFrame);

    bodySplitter = new QSplitter(Qt::Vertical, this);
    auto* contentWidget = new QWidget(this);
    bodySplitter->addWidget(contentWidget);

    auto* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(bodySplitter);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(mainLayout);

    auto* footButtonsSmall = new QVBoxLayout();
    footButtonsSmall->setSpacing(FOOT_BUTTONS_SPACING);
    footButtonsSmall->addWidget(emoteButton);
    footButtonsSmall->addWidget(fileButton);

    auto* mainFootLayout = new QHBoxLayout();
    mainFootLayout->addWidget(msgEdit);
    mainFootLayout->addLayout(footButtonsSmall);
    mainFootLayout->addSpacing(MAIN_FOOT_LAYOUT_SPACING);
    mainFootLayout->addWidget(sendButton);
    mainFootLayout->setSpacing(0);

    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->addWidget(searchForm);
    contentLayout->addWidget(dateInfo);
    contentLayout->addWidget(chatWidget);
    contentLayout->addLayout(mainFootLayout);

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    quoteAction = menu.addAction(QIcon(), QString(), QKeySequence(Qt::ALT | Qt::Key_Q), this,
                                 &GenericChatForm::quoteSelectedText);
#else
    quoteAction = menu.addAction(QIcon(), QString(), this, &GenericChatForm::quoteSelectedText,
                                 QKeySequence(Qt::ALT | Qt::Key_Q));
#endif
    addAction(quoteAction);
    menu.addSeparator();

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    goToCurrentDateAction = menu.addAction(QIcon(), QString(), QKeySequence(Qt::CTRL | Qt::Key_G),
                                           this, &GenericChatForm::goToCurrentDate);
#else
    goToCurrentDateAction = menu.addAction(QIcon(), QString(), this, &GenericChatForm::goToCurrentDate,
                                           QKeySequence(Qt::CTRL | Qt::Key_G));
#endif
    addAction(goToCurrentDateAction);

    menu.addSeparator();

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    searchAction = menu.addAction(QIcon(), QString(), QKeySequence(Qt::CTRL | Qt::Key_F), this,
                                  &GenericChatForm::searchFormShow);
#else
    searchAction = menu.addAction(QIcon(), QString(), this, &GenericChatForm::searchFormShow,
                                  QKeySequence(Qt::CTRL | Qt::Key_F));
#endif
    addAction(searchAction);

    menu.addSeparator();

    menu.addActions(chatWidget->actions());
    menu.addSeparator();

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    clearAction = menu.addAction(QIcon::fromTheme("edit-clear"), QString(),
                                 QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L), this,
                                 qOverload<>(&GenericChatForm::clearChatArea));
#else
    clearAction = menu.addAction(QIcon::fromTheme("edit-clear"), QString(), this,
                                 qOverload<>(&GenericChatForm::clearChatArea),
                                 QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L));
#endif
    addAction(clearAction);

    copyLinkAction = menu.addAction(QIcon(), QString(), this, &GenericChatForm::copyLink);
    menu.addSeparator();

    loadHistoryAction = menu.addAction(QIcon(), QString(), this, &GenericChatForm::onLoadHistory);
    exportChatAction = menu.addAction(QIcon::fromTheme("document-save"), QString(), this,
                                      &GenericChatForm::onExportChat);

    connect(chatWidget, &ChatWidget::customContextMenuRequested, this,
            &GenericChatForm::onChatContextMenuRequested);
    connect(chatWidget, &ChatWidget::firstVisibleLineChanged, this,
            &GenericChatForm::updateShowDateInfo);

    connect(searchForm, &SearchForm::searchInBegin, chatWidget, &ChatWidget::startSearch);
    connect(searchForm, &SearchForm::searchUp, chatWidget, &ChatWidget::onSearchUp);
    connect(searchForm, &SearchForm::searchDown, chatWidget, &ChatWidget::onSearchDown);
    connect(searchForm, &SearchForm::visibleChanged, chatWidget, &ChatWidget::removeSearchPhrase);
    connect(chatWidget, &ChatWidget::messageNotFoundShow, searchForm, &SearchForm::showMessageNotFound);

    connect(msgEdit, &ChatTextEdit::enterPressed, this, &GenericChatForm::onSendTriggered);

    connect(&style, &Style::themeReload, this, &GenericChatForm::reloadTheme);

    reloadTheme();

    fileFlyout->setFixedSize(FILE_FLYOUT_SIZE);
    fileFlyout->setParent(this);
    fileButton->installEventFilter(this);
    fileFlyout->installEventFilter(this);

    retranslateUi();
    Translator::registerHandler([this] { retranslateUi(); }, this);

    // update header on name/title change
    connect(chat, &Chat::displayedNameChanged, this, &GenericChatForm::setName);
}

GenericChatForm::~GenericChatForm()
{
    Translator::unregister(this);
#ifdef SPELL_CHECKING
    delete decorator;
#endif
    delete searchForm;
}

void GenericChatForm::adjustFileMenuPosition()
{
    const QPoint pos = fileButton->mapTo(bodySplitter, QPoint());
    const QSize size = fileFlyout->size();
    fileFlyout->move(pos.x() - size.width(), pos.y());
}

void GenericChatForm::showFileMenu()
{
    if (!fileFlyout->isShown() && !fileFlyout->isBeingShown()) {
        adjustFileMenuPosition();
    }

    fileFlyout->animateShow();
}

void GenericChatForm::hideFileMenu()
{
    if (fileFlyout->isShown() || fileFlyout->isBeingShown())
        fileFlyout->animateHide();
}

QDateTime GenericChatForm::getLatestTime() const
{
    if (chatLog.getFirstIdx() == chatLog.getNextIdx())
        return {};

    const auto shouldUseTimestamp = [this](ChatLogIdx idx) {
        if (chatLog.at(idx).getContentType() != ChatLogItem::ContentType::systemMessage) {
            return true;
        }

        const auto& message = chatLog.at(idx).getContentAsSystemMessage();
        switch (message.messageType) {
        case SystemMessageType::incomingCall:
        case SystemMessageType::outgoingCall:
        case SystemMessageType::callEnd:
        case SystemMessageType::unexpectedCallEnd:
            return true;
        case SystemMessageType::cleared:
        case SystemMessageType::titleChanged:
        case SystemMessageType::peerStateChange:
        case SystemMessageType::peerNameChanged:
        case SystemMessageType::userLeftConference:
        case SystemMessageType::userJoinedConference:
        case SystemMessageType::fileSendFailed:
        case SystemMessageType::messageSendFailed:
        case SystemMessageType::selfJoinedConference:
        case SystemMessageType::selfLeftConference:
            return false;
        }

        qWarning("Unexpected system message type %d", static_cast<int>(message.messageType));
        return false;
    };

    ChatLogIdx idx = chatLog.getNextIdx();
    while (idx > chatLog.getFirstIdx()) {
        idx = idx - 1;
        if (shouldUseTimestamp(idx)) {
            return chatLog.at(idx).getTimestamp();
        }
    }

    return {};
}

void GenericChatForm::reloadTheme()
{
    msgEdit->setStyleSheet(style.getStylesheet("msgEdit/msgEdit.qss", settings)
                           + fontToCss(settings.getChatMessageFont(), "QTextEdit"));

    emoteButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    fileButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    screenshotButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
    sendButton->setStyleSheet(style.getStylesheet(STYLE_PATH, settings));
}

void GenericChatForm::setName(const QString& newName)
{
    headWidget->setName(newName);
}

void GenericChatForm::show(ContentLayout* contentLayout_)
{
    contentLayout_->mainHead->layout()->addWidget(headWidget);
    headWidget->show();

    contentLayout_->mainContent->layout()->addWidget(this);
    QWidget::show();
}

void GenericChatForm::showEvent(QShowEvent* event)
{
    std::ignore = event;
    msgEdit->setFocus();
    headWidget->showCallConfirm();
}

bool GenericChatForm::event(QEvent* e)
{
    // If the user accidentally starts typing outside of the msgEdit, focus it automatically
    if (e->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(e);
        if ((ke->modifiers() == Qt::NoModifier || ke->modifiers() == Qt::ShiftModifier)
            && !ke->text().isEmpty()) {
            if (searchForm->isHidden()) {
                msgEdit->sendKeyEvent(ke);
                msgEdit->setFocus();
            } else {
                searchForm->insertEditor(ke->text());
                searchForm->setFocusEditor();
            }
        }
    }
    return QWidget::event(e);
}

void GenericChatForm::onChatContextMenuRequested(QPoint pos)
{
    auto* sender = static_cast<QWidget*>(QObject::sender());
    pos = sender->mapToGlobal(pos);

    // If we right-clicked on a link, give the option to copy it
    bool clickedOnLink = false;
    Text* clickedText = qobject_cast<Text*>(chatWidget->getContentFromGlobalPos(pos));
    if (clickedText != nullptr) {
        const QPointF scenePos = chatWidget->mapToScene(chatWidget->mapFromGlobal(pos));
        const QString linkTarget = clickedText->getLinkAt(scenePos);
        if (!linkTarget.isEmpty()) {
            clickedOnLink = true;
            copyLinkAction->setData(linkTarget);
        }
    }
    copyLinkAction->setVisible(clickedOnLink);

    menu.exec(pos);
}

void GenericChatForm::onSendTriggered()
{
    auto msg = msgEdit->toPlainText();

    const bool isAction = msg.startsWith(ChatForm::ACTION_PREFIX, Qt::CaseInsensitive);
    if (isAction) {
        msg.remove(0, ChatForm::ACTION_PREFIX.length());
    }

    if (msg.isEmpty()) {
        return;
    }

    msgEdit->setLastMessage(msg);
    msgEdit->clear();

    messageDispatcher.sendMessage(isAction, msg);
}


void GenericChatForm::onEmoteButtonClicked()
{
    // don't show the smiley selection widget if there are no smileys available
    if (smileyPack.getEmoticons().empty())
        return;

    EmoticonsWidget widget(smileyPack, settings, style);
    connect(&widget, &EmoticonsWidget::insertEmoticon, this, &GenericChatForm::onEmoteInsertRequested);
    widget.installEventFilter(this);

    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender != nullptr) {
        const QPoint pos =
            -QPoint(widget.sizeHint().width() / 2, widget.sizeHint().height()) - QPoint(0, 10);
        widget.exec(sender->mapToGlobal(pos));
    }
}

void GenericChatForm::onEmoteInsertRequested(QString str)
{
    // insert the emoticon
    QWidget* sender = qobject_cast<QWidget*>(QObject::sender());
    if (sender != nullptr)
        msgEdit->insertPlainText(str);

    msgEdit->setFocus(); // refocus so that we can continue typing
}

void GenericChatForm::onCopyLogClicked()
{
    chatWidget->copySelectedText();
}

void GenericChatForm::focusInput()
{
    msgEdit->setFocus();
}

void GenericChatForm::onChatMessageFontChanged(const QFont& font)
{
    // chat log
    chatWidget->fontChanged(font);
    chatWidget->forceRelayout();
    // message editor
    msgEdit->setStyleSheet(style.getStylesheet("msgEdit/msgEdit.qss", settings)
                           + fontToCss(font, "QTextEdit"));
}

void GenericChatForm::setColorizedNames(bool enable)
{
    chatWidget->setColorizedNames(enable);
}

void GenericChatForm::addSystemInfoMessage(const QDateTime& datetime, SystemMessageType messageType,
                                           SystemMessage::Args messageArgs)
{
    SystemMessage systemMessage;
    systemMessage.messageType = messageType;
    systemMessage.timestamp = datetime;
    systemMessage.args = std::move(messageArgs);
    chatLog.addSystemMessage(systemMessage);
}

QDateTime GenericChatForm::getTime(const ChatLine::Ptr& chatLine)
{
    if (chatLine) {
        auto* const timestamp = qobject_cast<Timestamp*>(chatLine->getContent(2));

        if (timestamp != nullptr) {
            return timestamp->getTime();
        }
        return {};
    }

    return {};
}


void GenericChatForm::clearChatArea()
{
    clearChatArea(/* confirm = */ true, /* inform = */ true);
}

void GenericChatForm::clearChatArea(bool confirm, bool inform)
{
    if (confirm) {
        const QMessageBox::StandardButton mboxResult =
            QMessageBox::question(this, tr("Confirmation"),
                                  tr("Are you sure that you want to clear all displayed messages?"),
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (mboxResult == QMessageBox::No) {
            return;
        }
    }

    chatWidget->clear();

    if (inform)
        addSystemInfoMessage(QDateTime::currentDateTime(), SystemMessageType::cleared, {});
}

void GenericChatForm::onSelectAllClicked()
{
    chatWidget->selectAll();
}

void GenericChatForm::hideEvent(QHideEvent* event)
{
    hideFileMenu();
    QWidget::hideEvent(event);
}

void GenericChatForm::resizeEvent(QResizeEvent* event)
{
    adjustFileMenuPosition();
    QWidget::resizeEvent(event);
}

bool GenericChatForm::eventFilter(QObject* object, QEvent* event)
{
    auto* ev = qobject_cast<EmoticonsWidget*>(object);
    if ((ev != nullptr) && event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        msgEdit->sendKeyEvent(key);
        msgEdit->setFocus();
        return false;
    }

    if (object != fileButton && object != fileFlyout)
        return false;

    auto* wObject = qobject_cast<QWidget*>(object);
    if ((wObject == nullptr) || !wObject->isEnabled())
        return false;

    switch (event->type()) {
    case QEvent::Enter:
        showFileMenu();
        break;

    case QEvent::Leave: {
        const QPoint flyPos = fileFlyout->mapToGlobal(QPoint());
        const QSize flySize = fileFlyout->size();

        const QPoint filePos = fileButton->mapToGlobal(QPoint());
        const QSize fileSize = fileButton->size();

        const QRect region = QRect(flyPos, flySize).united(QRect(filePos, fileSize));

        if (!region.contains(QCursor::pos()))
            hideFileMenu();

        break;
    }

    case QEvent::MouseButtonPress:
        hideFileMenu();
        break;

    default:
        break;
    }

    return false;
}

void GenericChatForm::quoteSelectedText()
{
    const QString selectedText = chatWidget->getSelectedText();

    if (selectedText.isEmpty())
        return;

    // forming pretty quote text
    // 1. insert "> " to the begining of quote;
    // 2. replace all possible line terminators with "\n> ";
    // 3. append new line to the end of quote.
    QString quote = selectedText;

    quote.insert(0, "> ");
    quote.replace(QRegularExpression(QString("\r\n|[\r\n\u2028\u2029]")), QString("\n> "));
    quote.append("\n");

    msgEdit->append(quote);
}

/**
 * @brief Callback of GenericChatForm::copyLinkAction
 */
void GenericChatForm::copyLink()
{
    const QString linkText = copyLinkAction->data().toString();
    auto* clipboard = QApplication::clipboard();
    clipboard->setText(linkText, QClipboard::Clipboard);
    if (clipboard->supportsSelection()) {
        clipboard->setText(linkText, QClipboard::Selection);
    }
}

void GenericChatForm::searchFormShow()
{
    if (searchForm->isHidden()) {
        searchForm->show();
        searchForm->setFocusEditor();
    }
}

void GenericChatForm::onLoadHistory()
{
    LoadHistoryDialog dlg(&chatLog);
    if (dlg.exec() != 0) {
        chatWidget->jumpToDate(dlg.getFromDate().date());
    }
}

void GenericChatForm::onExportChat()
{
    const QString path = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save chat log"));
    if (path.isEmpty()) {
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QString buffer;
    for (auto i = chatLog.getFirstIdx(); i < chatLog.getNextIdx(); ++i) {
        const auto& item = chatLog.at(i);
        if (item.getContentType() != ChatLogItem::ContentType::message) {
            continue;
        }

        QString timestamp = item.getTimestamp().time().toString("hh:mm:ss");
        QString datestamp = item.getTimestamp().date().toString("yyyy-MM-dd");
        QString author = item.getDisplayName();

        buffer = buffer
                 % QString{datestamp % '\t' % timestamp % '\t' % author % '\t'
                           % item.getContentAsMessage().message.content % '\n'};
    }
    file.write(buffer.toUtf8());
    file.close();
}

void GenericChatForm::goToCurrentDate()
{
    chatWidget->jumpToIdx(chatLog.getNextIdx());
}

void GenericChatForm::updateShowDateInfo(const ChatLine::Ptr& topLine)
{
    const auto date = getTime(topLine);

    if (date.isValid()) {
        dateInfo->setText(QStringLiteral("<b>%1<\b>").arg(date.toString(settings.getDateFormat())));
    }
}

void GenericChatForm::retranslateUi()
{
    sendButton->setToolTip(tr("Send message"));
    emoteButton->setToolTip(tr("Smileys"));
    fileButton->setToolTip(tr("Send file(s)"));
    screenshotButton->setToolTip(tr("Send a screenshot"));
    clearAction->setText(tr("Clear displayed messages"));
    quoteAction->setText(tr("Quote selected text"));
    copyLinkAction->setText(tr("Copy link address"));
    searchAction->setText(tr("Search in text"));
    goToCurrentDateAction->setText(tr("Go to current date"));
    loadHistoryAction->setText(tr("Load chat history..."));
    exportChatAction->setText(tr("Export to file"));
}
