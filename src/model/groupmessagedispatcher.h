/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/core/icoregroupmessagesender.h"
#include "src/core/icoreidhandler.h"
#include "src/model/group.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/message.h"

#include <QObject>
#include <QString>

class Settings;

class GroupMessageDispatcher : public IMessageDispatcher
{
    Q_OBJECT
public:
    GroupMessageDispatcher(Group& g_, MessageProcessor processor, ICoreIdHandler& idHandler,
                           ICoreGroupMessageSender& messageSender, Settings& settings);

    std::pair<DispatchedMessageId, DispatchedMessageId> sendMessage(bool isAction,
                                                                     const QString& content) override;

    std::pair<DispatchedMessageId, DispatchedMessageId> sendPrivateMessage(uint32_t peerId,
                                                                           bool isAction,
                                                                           const QString& content);

    void onMessageReceived(const ToxPk& sender, bool isAction, const QString& content);
    void onPrivateMessageReceived(const ToxPk& sender, bool isAction, const QString& content);

private:
    Group& group;
    MessageProcessor processor;
    ICoreIdHandler& idHandler;
    ICoreGroupMessageSender& messageSender;
    Settings& settings;
    DispatchedMessageId nextMessageId{0};
};
