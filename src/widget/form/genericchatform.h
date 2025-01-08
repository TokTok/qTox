/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/chatlog/chatmessage.h"
#include "src/core/toxpk.h"
#include "src/model/ichatlog.h"

#include <QMenu>
#include <QWidget>

/**
 * Spacing in px inserted when the author of the last message changes
 * @note Why the hell is this a thing? surely the different font is enough?
 *        - Even a different font is not enough – TODO #1307 ~~zetok
 */

class Chat;
class ChatFormHeader;
class ChatTextEdit;
class ChatWidget;
class ContentLayout;
class CroppingLabel;
class DocumentCache;
class FlyoutOverlayWidget;
class FriendList;
class GenericNetCamView;
class IMessageBoxManager;
class IMessageDispatcher;
class MaskablePixmapWidget;
class QLabel;
class QPushButton;
class QSplitter;
class QToolButton;
class QVBoxLayout;
class SearchForm;
class Settings;
class SmileyPack;
class Style;
class Widget;
struct Message;

namespace Ui {
class MainWindow;
}

#ifdef SPELL_CHECKING
namespace Sonnet {
class SpellCheckDecorator;
}
#endif

class GenericChatForm : public QWidget
{
    Q_OBJECT
public:
    GenericChatForm(const Core& core_, const Chat* chat, IChatLog& chatLog_,
                    IMessageDispatcher& messageDispatcher_, DocumentCache& documentCache,
                    SmileyPack& smileyPack, Settings& settings, Style& style,
                    IMessageBoxManager& messageBoxmanager, FriendList& friendList,
                    ConferenceList& conferenceList, ImageLoader& imageLoader, QWidget* parent_ = nullptr);
    ~GenericChatForm() override;

    void setName(const QString& newName);
    virtual void show(ContentLayout* contentLayout_);

    void addSystemInfoMessage(const QDateTime& datetime, SystemMessageType messageType,
                              SystemMessage::Args messageArgs);
    QString resolveToxPk(const ToxPk& pk);
    QDateTime getLatestTime() const;

signals:
    void messageInserted();

public slots:
    void clearChatArea();
    void focusInput();
    void onChatMessageFontChanged(const QFont& font);
    void setColorizedNames(bool enable);
    virtual void reloadTheme();

protected slots:
    void onChatContextMenuRequested(QPoint pos);
    virtual void onScreenshotClicked() = 0;
    void onSendTriggered();
    virtual void onAttachClicked() = 0;
    void onEmoteButtonClicked();
    void onEmoteInsertRequested(QString str);
    void onCopyLogClicked();
    void clearChatArea(bool confirm, bool inform);
    void onSelectAllClicked();
    void showFileMenu();
    void hideFileMenu();
    void quoteSelectedText();
    void copyLink();
    void onLoadHistory();
    void onExportChat();
    void searchFormShow();
    void updateShowDateInfo(const ChatLine::Ptr& prevLine, const ChatLine::Ptr& topLine);
    void goToCurrentDate();

private:
    void retranslateUi();
    QDateTime getTime(const ChatLine::Ptr& chatLine) const;

protected:
    void adjustFileMenuPosition();
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool event(QEvent* event) final;
    void resizeEvent(QResizeEvent* event) final;
    bool eventFilter(QObject* object, QEvent* event) final;

protected:
    const Core& core;
    bool audioInputFlag;
    bool audioOutputFlag;
    int curRow;

    QAction* clearAction;
    QAction* quoteAction;
    QAction* copyLinkAction;
    QAction* searchAction;
    QAction* goToCurrentDateAction;
    QAction* loadHistoryAction;
    QAction* exportChatAction;

    QMenu menu;

    QVBoxLayout* contentLayout;
    QPushButton* emoteButton;
    QPushButton* fileButton;
    QPushButton* screenshotButton;
    QPushButton* sendButton;

    QSplitter* bodySplitter;

    ChatFormHeader* headWidget;

    SearchForm* searchForm;
    QLabel* dateInfo;
    ChatWidget* chatWidget;
    ChatTextEdit* msgEdit;
#ifdef SPELL_CHECKING
    Sonnet::SpellCheckDecorator* decorator{nullptr};
#endif
    FlyoutOverlayWidget* fileFlyout;
    Widget* parent;

    IChatLog& chatLog;
    IMessageDispatcher& messageDispatcher;
    SmileyPack& smileyPack;
    Settings& settings;
    Style& style;
    FriendList& friendList;
    ConferenceList& conferenceList;
};
