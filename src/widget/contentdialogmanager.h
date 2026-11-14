/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2018-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "contentdialog.h"

#include "src/core/chatid.h"
#include "src/core/conferenceid.h"
#include "src/core/groupid.h"
#include "src/core/toxpk.h"
#include "src/model/dialogs/idialogsmanager.h"

#include <QObject>

/**
 * @brief Manage all content dialogs
 */
class ContentDialogManager : public QObject, public IDialogsManager
{
    Q_OBJECT
public:
    explicit ContentDialogManager(FriendList& friendList);
    ContentDialog* current();
    bool chatWidgetExists(const ChatId& chatId);
    void focusChat(const ChatId& chatId);
    void updateFriendStatus(const ToxPk& friendPk);
    void updateConferenceStatus(const ConferenceId& conferenceId);
    void updateGroupStatus(const GroupId& groupId);
    bool isChatActive(const ChatId& chatId);
    ContentDialog* getFriendDialog(const ToxPk& friendPk) const;
    ContentDialog* getConferenceDialog(const ConferenceId& conferenceId) const;
    ContentDialog* getGroupDialog(const GroupId& groupId) const;

    IDialogs* getFriendDialogs(const ToxPk& friendPk) const override;
    IDialogs* getConferenceDialogs(const ConferenceId& conferenceId) const override;
    IDialogs* getGroupDialogs(const GroupId& groupId) const override;

    FriendWidget* addFriendToDialog(ContentDialog* dialog, std::shared_ptr<FriendChatroom> chatroom,
                                    GenericChatForm* form);
    ConferenceWidget* addConferenceToDialog(ContentDialog* dialog,
                                            std::shared_ptr<ConferenceRoom> chatroom,
                                            GenericChatForm* form);
    GroupWidget* addGroupToDialog(ContentDialog* dialog, std::shared_ptr<GroupRoom> chatroom,
                                  GenericChatForm* form);

    void addContentDialog(ContentDialog& dialog);

private slots:
    void onDialogClose();
    void onDialogActivate();

private:
    static ContentDialog*
    focusDialog(const ChatId& id,
                const QHash<std::reference_wrapper<const ChatId>, ContentDialog*>& list);

    ContentDialog* currentDialog = nullptr;

    QHash<std::reference_wrapper<const ChatId>, ContentDialog*> chatDialogs;
    FriendList& friendList;
};
