/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "conference.h"
#include "friend.h"
#include "src/core/chatid.h"
#include "src/core/conferenceid.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"

#include <cassert>

#include <QDebug>

namespace {
const int MAX_CONFERENCE_TITLE_LENGTH = 128;
} // namespace

Conference::Conference(int conferenceId_, const ConferenceId persistentConferenceId,
                       const QString& name, bool isAvConference, const QString& selfName_,
                       ICoreConferenceQuery& conferenceQuery_, ICoreIdHandler& idHandler_,
                       FriendList& friendList_)
    : conferenceQuery(conferenceQuery_)
    , idHandler(idHandler_)
    , selfName{selfName_}
    , title{name}
    , toxConferenceNum(conferenceId_)
    , conferenceId{persistentConferenceId}
    , avConference{isAvConference}
    , friendList{friendList_}
{
    // in conferences, we only notify on messages containing your name <-- dumb
    // sound notifications should be on all messages, but system popup notification
    // on naming is appropriate
    hasNewMessages = 0;
    userWasMentioned = 0;
    regeneratePeerList();
}

void Conference::setName(const QString& newTitle)
{
    const QString shortTitle = newTitle.left(MAX_CONFERENCE_TITLE_LENGTH);
    if (!shortTitle.isEmpty() && title != shortTitle) {
        title = shortTitle;
        emit displayedNameChanged(title);
        emit titleChangedByUser(title);
        emit titleChanged(selfName, title);
    }
}

void Conference::setTitle(const QString& author, const QString& newTitle)
{
    const QString shortTitle = newTitle.left(MAX_CONFERENCE_TITLE_LENGTH);
    if (!shortTitle.isEmpty() && title != shortTitle) {
        title = shortTitle;
        emit displayedNameChanged(title);
        emit titleChanged(author, title);
    }
}

QString Conference::getName() const
{
    return title;
}

QString Conference::getDisplayedName() const
{
    return getName();
}

QString Conference::getDisplayedName(const ToxPk& contact) const
{
    return resolveToxPk(contact);
}

void Conference::regeneratePeerList()
{
    // NOTE: there's a bit of a race here. Core emits a signal for both conferencePeerlistChanged and
    // conferencePeerNameChanged back-to-back when a peer joins our conference. If we get both before we
    // process this slot, core->getConferencePeerNames will contain the new peer name, and we'll ignore
    // the name changed signal, and emit a single userJoined with the correct name. But, if we
    // receive the name changed signal a little later, we will emit userJoined before we have their
    // username, using just their ToxPk, then shortly after emit another peerNameChanged signal.
    // This can cause double-updated to UI and chat log, but is unavoidable given the API of toxcore.
    QStringList peers = conferenceQuery.getConferencePeerNames(toxConferenceNum);
    const auto oldPeerNames = peerDisplayNames;
    peerDisplayNames.clear();
    const int nPeers = peers.size();
    for (int i = 0; i < nPeers; ++i) {
        const auto pk = conferenceQuery.getConferencePeerPk(toxConferenceNum, i);
        if (pk == idHandler.getSelfPublicKey()) {
            peerDisplayNames[pk] = idHandler.getUsername();
        } else {
            peerDisplayNames[pk] = friendList.decideNickname(pk, peers[i]);
        }
    }
    for (const auto& pk : oldPeerNames.keys()) {
        if (!peerDisplayNames.contains(pk)) {
            emit userLeft(pk, oldPeerNames.value(pk));
        }
    }
    for (const auto& pk : peerDisplayNames.keys()) {
        if (!oldPeerNames.contains(pk)) {
            emit userJoined(pk, peerDisplayNames.value(pk));
        }
    }
    for (const auto& pk : peerDisplayNames.keys()) {
        if (oldPeerNames.contains(pk) && oldPeerNames.value(pk) != peerDisplayNames.value(pk)) {
            emit peerNameChanged(pk, oldPeerNames.value(pk), peerDisplayNames.value(pk));
        }
    }
    if (oldPeerNames.size() != nPeers) {
        emit numPeersChanged(nPeers);
    }
}

void Conference::updateUsername(ToxPk pk, const QString newName)
{
    const QString displayName = friendList.decideNickname(pk, newName);
    assert(peerDisplayNames.contains(pk));
    if (peerDisplayNames[pk] != displayName) {
        // there could be no actual change even if their username changed due to an alias being set
        const auto oldName = peerDisplayNames[pk];
        peerDisplayNames[pk] = displayName;
        emit peerNameChanged(pk, oldName, displayName);
    }
}

bool Conference::isAvConference() const
{
    return avConference;
}

uint32_t Conference::getId() const
{
    return toxConferenceNum;
}

const ConferenceId& Conference::getPersistentId() const
{
    return conferenceId;
}

int Conference::getPeersCount() const
{
    return peerDisplayNames.size();
}

/**
 * @brief Gets the PKs and names of all peers
 * @return PKs and names of all peers, including our own PK and name
 */
const QMap<ToxPk, QString>& Conference::getPeerList() const
{
    return peerDisplayNames;
}

void Conference::setEventFlag(bool f)
{
    hasNewMessages = f;
}

bool Conference::getEventFlag() const
{
    return hasNewMessages;
}

void Conference::setMentionedFlag(bool f)
{
    userWasMentioned = f;
}

bool Conference::getMentionedFlag() const
{
    return userWasMentioned;
}

QString Conference::resolveToxPk(const ToxPk& id) const
{
    auto it = peerDisplayNames.find(id);

    if (it != peerDisplayNames.end()) {
        return *it;
    }

    return QString();
}

void Conference::setSelfName(const QString& name)
{
    selfName = name;
}

QString Conference::getSelfName() const
{
    return selfName;
}
