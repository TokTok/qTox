/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "chat.h"

#include "src/core/chatid.h"
#include "src/core/groupid.h"
#include "src/core/icoregroupquery.h"
#include "src/core/icoreidhandler.h"
#include "src/core/toxpk.h"

#include <QMap>
#include <QObject>
#include <QStringList>

class FriendList;

class Group : public Chat
{
    Q_OBJECT
public:
    Group(int groupId_, const GroupId persistentGroupId, QString name, bool isAvGroupchat,
          QString selfName_, ICoreGroupQuery& groupQuery_, ICoreIdHandler& idHandler_,
          FriendList& friendList);
    bool isAvGroupchat() const;
    uint32_t getId() const override;
    const GroupId& getPersistentId() const override;
    int getPeersCount() const;
    void regeneratePeerList();
    const QMap<ToxPk, QString>& getPeerList() const;
    bool peerHasNickname(ToxPk pk);

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void updateUsername(ToxPk pk, const QString newName);
    void setName(const QString& newTitle) override;
    void setTitle(const QString& author, const QString& newTitle);
    QString getName() const;
    QString getDisplayedName() const override;
    QString getDisplayedName(const ToxPk& contact) const override;
    QString resolveToxPk(const ToxPk& id) const;
    void setSelfName(const QString& name);
    QString getSelfName() const;

signals:
    void titleChangedByUser(const QString& title);
    void titleChanged(const QString& author, const QString& title);
    void userJoined(const ToxPk& user, const QString& name);
    void userLeft(const ToxPk& user, const QString& name);
    void numPeersChanged(int numPeers);
    void peerNameChanged(const ToxPk& peer, const QString& oldName, const QString& newName);

private:
    ICoreGroupQuery& groupQuery;
    ICoreIdHandler& idHandler;
    QString selfName;
    QString title;
    QMap<ToxPk, QString> peerDisplayNames;
    bool hasNewMessages;
    bool userWasMentioned;
    int toxGroupNum;
    const GroupId groupId;
    bool avGroupchat;
    FriendList& friendList;
};
