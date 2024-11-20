/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "chat.h"
#include "src/core/chatid.h"
#include "src/core/extension.h"
#include "src/core/toxpk.h"
#include "src/model/status.h"
#include <QObject>
#include <QString>

class Friend : public Chat
{
    Q_OBJECT
public:
    Friend(uint32_t friendId_, ToxPk friendPk_, QString userAlias_ = {}, const QString& userName_ = {});
    Friend(const Friend& other) = delete;
    Friend& operator=(const Friend& other) = delete;

    void setName(const QString& name) override;
    void setAlias(const QString& alias);
    QString getDisplayedName() const override;
    QString getDisplayedName(const ToxPk& contact) const override;
    bool hasAlias() const;
    QString getUserName() const;
    void setStatusMessage(const QString& message);
    QString getStatusMessage() const;

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    const ToxPk& getPublicKey() const;
    uint32_t getId() const override;
    const ChatId& getPersistentId() const override;

    void finishNegotiation();
    void setStatus(Status::Status s);
    Status::Status getStatus() const;

    void setExtendedMessageSupport(bool supported);
    ExtensionSet getSupportedExtensions() const;

signals:
    void nameChanged(const ToxPk& friendId, const QString& name);
    void aliasChanged(const ToxPk& friendId, QString alias);
    void statusChanged(const ToxPk& friendId, Status::Status status);
    void onlineOfflineChanged(const ToxPk& friendId, bool isOnline);
    void statusMessageChanged(const ToxPk& friendId, const QString& message);
    void extensionSupportChanged(ExtensionSet extensions);
    void loadChatHistory();

public slots:
    void onNegotiationComplete();

private:
    QString userName;
    QString userAlias;
    QString statusMessage;
    ToxPk friendPk;
    uint32_t friendId;
    bool hasNewEvents;
    Status::Status friendStatus;
    bool isNegotiating;
    ExtensionSet supportedExtensions;
};
