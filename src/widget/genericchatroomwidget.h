/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatitemwidget.h"

class CroppingLabel;
class MaskablePixmapWidget;
class QVBoxLayout;
class QHBoxLayout;
class ContentLayout;
class Friend;
class Conference;
class Settings;
class Chat;
class Style;

class GenericChatroomWidget : public GenericChatItemWidget
{
    Q_OBJECT
public:
    GenericChatroomWidget(bool compact, Settings& settings, Style& style, QWidget* parent = nullptr);

public slots:
    virtual void setAsActiveChatroom() = 0;
    virtual void setAsInactiveChatroom() = 0;
    virtual void updateStatusLight() = 0;
    virtual void resetEventFlags() = 0;
    virtual QString getStatusString() const = 0;
    virtual const Chat* getChat() const = 0;
    virtual const Friend* getFriend() const
    {
        return nullptr;
    }
    virtual Conference* getConference() const
    {
        return nullptr;
    }

    bool eventFilter(QObject* object, QEvent* event) final;

    bool isActive();

    void setName(const QString& name);
    void setStatusMsg(const QString& status);
    QString getStatusMsg() const;
    QString getTitle() const;

    void activate();
    void compactChange(bool compact);
    void reloadTheme() override;

signals:
    void chatroomWidgetClicked(GenericChatroomWidget* widget);
    void newWindowOpened(GenericChatroomWidget* widget);
    void middleMouseClicked();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void setActive(bool active);

protected:
    QPoint dragStartPos;
    QColor lastColor;
    QHBoxLayout* mainLayout = nullptr;
    QVBoxLayout* textLayout = nullptr;
    MaskablePixmapWidget* avatar;
    CroppingLabel* statusMessageLabel;
    bool active;
    Settings& settings;
    Style& style;
};
