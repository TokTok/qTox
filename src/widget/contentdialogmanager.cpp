/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "contentdialogmanager.h"

#include "src/conferencelist.h"
#include "src/friendlist.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/widget/conferencewidget.h"
#include "src/widget/friendwidget.h"

#include <tuple>

namespace {
void removeDialog(ContentDialog* dialog,
                  QHash<std::reference_wrapper<const ChatId>, ContentDialog*>& dialogs)
{
    for (auto it = dialogs.begin(); it != dialogs.end();) {
        if (*it == dialog) {
            it = dialogs.erase(it);
        } else {
            ++it;
        }
    }
}
} // namespace

ContentDialogManager::ContentDialogManager(FriendList& friendList_)
    : friendList{friendList_}
{
}

ContentDialog* ContentDialogManager::current()
{
    return currentDialog;
}

bool ContentDialogManager::chatWidgetExists(const ChatId& chatId)
{
    const auto dialog = chatDialogs.value(chatId, nullptr);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->hasChat(chatId);
}

FriendWidget* ContentDialogManager::addFriendToDialog(ContentDialog* dialog,
                                                      std::shared_ptr<FriendChatroom> chatroom,
                                                      GenericChatForm* form)
{
    auto friendWidget = dialog->addFriend(chatroom, form);
    const auto& friendPk = friendWidget->getFriend()->getPublicKey();

    ContentDialog* lastDialog = getFriendDialog(friendPk);
    if (lastDialog != nullptr) {
        lastDialog->removeFriend(friendPk);
    }

    chatDialogs[friendPk] = dialog;
    return friendWidget;
}

ConferenceWidget* ContentDialogManager::addConferenceToDialog(ContentDialog* dialog,
                                                              std::shared_ptr<ConferenceRoom> chatroom,
                                                              GenericChatForm* form)
{
    auto conferenceWidget = dialog->addConference(chatroom, form);
    const auto& conferenceId = conferenceWidget->getConference()->getPersistentId();

    ContentDialog* lastDialog = getConferenceDialog(conferenceId);
    if (lastDialog != nullptr) {
        lastDialog->removeConference(conferenceId);
    }

    chatDialogs[conferenceId] = dialog;
    return conferenceWidget;
}

void ContentDialogManager::focusChat(const ChatId& chatId)
{
    auto dialog = focusDialog(chatId, chatDialogs);
    if (dialog != nullptr) {
        dialog->focusChat(chatId);
    }
}

/**
 * @brief Focus the dialog if it exists.
 * @param id User Id.
 * @param list List with dialogs
 * @return ContentDialog if found, nullptr otherwise
 */
ContentDialog* ContentDialogManager::focusDialog(
    const ChatId& id, const QHash<std::reference_wrapper<const ChatId>, ContentDialog*>& list)
{
    auto iter = list.find(id);
    if (iter == list.end()) {
        return nullptr;
    }

    ContentDialog* dialog = *iter;
    if (dialog->windowState() & Qt::WindowMinimized != 0u) {
        dialog->showNormal();
    }

    dialog->raise();
    dialog->activateWindow();
    return dialog;
}

void ContentDialogManager::updateFriendStatus(const ToxPk& friendPk)
{
    auto dialog = chatDialogs.value(friendPk);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateChatStatusLight(friendPk);
    if (dialog->isChatActive(friendPk)) {
        dialog->updateTitleAndStatusIcon();
    }

    Friend* f = friendList.findFriend(friendPk);
    dialog->updateFriendStatus(friendPk, f->getStatus());
}

void ContentDialogManager::updateConferenceStatus(const ConferenceId& conferenceId)
{
    auto dialog = chatDialogs.value(conferenceId);
    if (dialog == nullptr) {
        return;
    }

    dialog->updateChatStatusLight(conferenceId);
    if (dialog->isChatActive(conferenceId)) {
        dialog->updateTitleAndStatusIcon();
    }
}

bool ContentDialogManager::isChatActive(const ChatId& chatId)
{
    const auto dialog = chatDialogs.value(chatId);
    if (dialog == nullptr) {
        return false;
    }

    return dialog->isChatActive(chatId);
}

ContentDialog* ContentDialogManager::getFriendDialog(const ToxPk& friendPk) const
{
    return chatDialogs.value(friendPk);
}

ContentDialog* ContentDialogManager::getConferenceDialog(const ConferenceId& conferenceId) const
{
    return chatDialogs.value(conferenceId);
}

void ContentDialogManager::addContentDialog(ContentDialog& dialog)
{
    currentDialog = &dialog;
    connect(&dialog, &ContentDialog::willClose, this, &ContentDialogManager::onDialogClose);
    connect(&dialog, &ContentDialog::activated, this, &ContentDialogManager::onDialogActivate);
}

void ContentDialogManager::onDialogActivate()
{
    ContentDialog* dialog = qobject_cast<ContentDialog*>(sender());
    currentDialog = dialog;
}

void ContentDialogManager::onDialogClose()
{
    ContentDialog* dialog = qobject_cast<ContentDialog*>(sender());
    if (currentDialog == dialog) {
        currentDialog = nullptr;
    }

    removeDialog(dialog, chatDialogs);
}

IDialogs* ContentDialogManager::getFriendDialogs(const ToxPk& friendPk) const
{
    return getFriendDialog(friendPk);
}

IDialogs* ContentDialogManager::getConferenceDialogs(const ConferenceId& conferenceId) const
{
    return getConferenceDialog(conferenceId);
}
