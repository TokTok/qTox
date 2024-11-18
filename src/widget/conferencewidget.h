/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "genericchatroomwidget.h"

#include "src/core/conferenceid.h"
#include "src/model/chatroom/conferenceroom.h"
#include "src/model/friendlist/ifriendlistitem.h"

#include <memory>

class Settings;
class Style;

class ConferenceWidget final : public GenericChatroomWidget, public IFriendListItem
{
    Q_OBJECT
public:
    ConferenceWidget(std::shared_ptr<ConferenceRoom> chatroom_, bool compact, Settings& settings,
                Style& style);
    ~ConferenceWidget();
    void setAsInactiveChatroom() final;
    void setAsActiveChatroom() final;
    void updateStatusLight() final;
    void resetEventFlags() final;
    QString getStatusString() const final;
    Conference* getConference() const final;
    const Chat* getChat() const final;
    void setName(const QString& name);
    void editName();

    bool isFriend() const final;
    bool isConference() const final;
    QString getNameItem() const final;
    bool isOnline() const final;
    bool widgetIsVisible() const final;
    QDateTime getLastActivity() const final;
    QWidget* getWidget() final;
    void setWidgetVisible(bool visible) final;

signals:
    void conferenceWidgetClicked(ConferenceWidget* widget);
    void removeConference(const ConferenceId& conferenceId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) final;
    void mousePressEvent(QMouseEvent* event) final;
    void mouseMoveEvent(QMouseEvent* event) final;
    void dragEnterEvent(QDragEnterEvent* ev) override;
    void dragLeaveEvent(QDragLeaveEvent* ev) override;
    void dropEvent(QDropEvent* ev) override;

private slots:
    void retranslateUi();
    void updateTitle(const QString& author, const QString& newName);
    void updateUserCount(int numPeers);

public:
    ConferenceId conferenceId;

private:
    std::shared_ptr<ConferenceRoom> chatroom;
};
