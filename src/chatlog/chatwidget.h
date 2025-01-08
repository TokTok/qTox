/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QDateTime>
#include <QGraphicsView>
#include <QMargins>

#include "chatline.h"
#include "chatmessage.h"
#include "src/model/ichatlog.h"

class ChatLineContent;
class ChatLineStorage;
class IMessageBoxManager;
class QGraphicsRectItem;
class QGraphicsScene;
class QMouseEvent;
class QTimer;
class ImageLoader;
class Settings;
class SmileyPack;
class Style;
struct ToxFile;

static const size_t DEF_NUM_MSG_TO_LOAD = 100;
class ChatWidget : public QGraphicsView
{
    Q_OBJECT
public:
    ChatWidget(IChatLog& chatLog_, const Core& core_, DocumentCache& documentCache,
               SmileyPack& smileyPack, Settings& settings, Style& style,
               IMessageBoxManager& messageBoxManager, ImageLoader& imageLoader, QWidget* parent = nullptr);
    ~ChatWidget() override;

    void insertChatlines(std::map<ChatLogIdx, ChatLine::Ptr> chatLines);
    void clearSelection();
    void clear();
    void copySelectedText(bool toSelectionBuffer = false) const;
    void setTypingNotificationVisible(bool visible);
    void setTypingNotificationName(const QString& displayName);
    void scrollToLine(ChatLine::Ptr line);
    void selectAll();
    void fontChanged(const QFont& font);

    QString getSelectedText() const;

    bool isEmpty() const;
    bool hasTextToBeCopied() const;
    ChatLineContent* getContentFromGlobalPos(QPoint pos) const;
    const uint repNameAfter = 5 * 60;

    void setColorizedNames(bool enable)
    {
        colorizeNames = enable;
    }
    void jumpToDate(QDate date);
    void jumpToIdx(ChatLogIdx idx);

signals:
    void selectionChanged();
    void firstVisibleLineChanged(const ChatLine::Ptr& prevLine, const ChatLine::Ptr& firstLine);

    void messageNotFoundShow(SearchDirection direction);
    void renderFinished();
public slots:
    void forceRelayout();
    void reloadTheme();

    void startSearch(const QString& phrase, const ParameterSearch& parameter);
    void onSearchUp(const QString& phrase, const ParameterSearch& parameter);
    void onSearchDown(const QString& phrase, const ParameterSearch& parameter);
    void handleSearchResult(SearchResult result, SearchDirection direction);
    void removeSearchPhrase();

private slots:
    void onSelectionTimerTimeout();
    void onWorkerTimeout();
    void onMultiClickTimeout();

    void onMessageUpdated(ChatLogIdx idx);
    void renderMessage(ChatLogIdx idx);
    void renderMessages(ChatLogIdx begin, ChatLogIdx end);

    void setRenderedWindowStart(ChatLogIdx begin);
    void setRenderedWindowEnd(ChatLogIdx end);

    void onRenderFinished();
    void onScrollValueChanged(int value);

protected:
    QRectF calculateSceneRect() const;
    QRect getVisibleRect() const;
    ChatLineContent* getContentFromPos(QPointF scenePos) const;

    void layout(int start, int end, qreal width);
    bool isOverSelection(QPointF scenePos) const;
    bool stickToBottom() const;

    qreal useableWidth() const;

    void updateSceneRect();
    void checkVisibility();
    void scrollToBottom();
    void startResizeWorker();

    void mouseDoubleClickEvent(QMouseEvent* ev) final;
    void mousePressEvent(QMouseEvent* ev) final;
    void mouseReleaseEvent(QMouseEvent* ev) final;
    void mouseMoveEvent(QMouseEvent* ev) final;
    void scrollContentsBy(int dx, int dy) final;
    void resizeEvent(QResizeEvent* ev) final;
    void showEvent(QShowEvent* event) final;
    void hideEvent(QHideEvent* event) final;
    void focusInEvent(QFocusEvent* ev) final;
    void focusOutEvent(QFocusEvent* ev) final;
    void wheelEvent(QWheelEvent* event) final;

    void updateMultiSelectionRect();
    void updateTypingNotification();
    void updateBusyNotification();

    ChatLine::Ptr findLineByPosY(qreal yPos) const;

    void removeLines(ChatLogIdx begin, ChatLogIdx end);

private:
    void retranslateUi();
    bool isActiveFileTransfer(ChatLine::Ptr l);
    void handleMultiClickEvent();
    void moveSelectionRectUpIfSelected(int offset);
    void moveSelectionRectDownIfSelected(int offset);
    void movePreciseSelectionDown(int offset);
    void movePreciseSelectionUp(int offset);
    void moveMultiSelectionUp(int offset);
    void moveMultiSelectionDown(int offset);
    void setTypingNotification();

    void renderItem(const ChatLogItem& item, bool hideName, bool colorizeNames_,
                    ChatLine::Ptr& chatMessage);
    void renderFile(QString displayName, ToxFile file, bool isSelf, QDateTime timestamp,
                    ChatLine::Ptr& chatMessage);
    bool needsToHideName(ChatLogIdx idx, bool prevIdxRendered) const;
    bool shouldRenderMessage(ChatLogIdx idx) const;
    void disableSearchText();

private:
    enum class SelectionMode
    {
        None,
        Precise,
        Multi,
    };

    enum class AutoScrollDirection
    {
        NoDirection,
        Up,
        Down,
    };

    QAction* copyAction = nullptr;
    QAction* selectAllAction = nullptr;
    QGraphicsScene* scene = nullptr;
    QGraphicsScene* busyScene = nullptr;
    QList<ChatLine::Ptr> visibleLines;
    ChatLine::Ptr typingNotification;
    ChatLine::Ptr busyNotification;

    // selection

    // For the time being we store these selection indexes as ChatLine::Ptrs. In
    // order to do multi-selection we do an O(n) search in the chatline storage
    // to determine the index. This is inefficient but correct with the moving
    // window of storage. If this proves to cause performance issues we can move
    // this responsibility into ChatlineStorage and have it coordinate the
    // shifting of indexes
    ChatLine::Ptr selClickedRow; // These 4 are only valid while selectionMode != None
    int selClickedCol = -1;
    ChatLine::Ptr selFirstRow;
    ChatLine::Ptr selLastRow;
    QColor selectionRectColor;
    SelectionMode selectionMode = SelectionMode::None;
    QPointF clickPos;
    QGraphicsRectItem* selGraphItem = nullptr;
    QTimer* selectionTimer = nullptr;
    QTimer* workerTimer = nullptr;
    QTimer* multiClickTimer = nullptr;
    AutoScrollDirection selectionScrollDir = AutoScrollDirection::NoDirection;
    int clickCount = 0;
    QPoint lastClickPos;
    Qt::MouseButton lastClickButton;

    // worker vars
    size_t workerLastIndex = 0;
    bool workerStb = false;
    ChatLine::Ptr workerAnchorLine;

    // layout
    QMargins margins = QMargins(10, 10, 10, 10);
    qreal lineSpacing = 5.0;

    IChatLog& chatLog;
    bool colorizeNames = false;
    SearchPos searchPos;
    const Core& core;
    bool scrollMonitoringEnabled = true;

    std::unique_ptr<ChatLineStorage> chatLineStorage;

    std::vector<std::function<void(void)>> renderCompletionFns;
    DocumentCache& documentCache;
    SmileyPack& smileyPack;
    Settings& settings;
    Style& style;
    IMessageBoxManager& messageBoxManager;
    ImageLoader& imageLoader;
};
