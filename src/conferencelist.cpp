/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "conferencelist.h"
#include "src/core/core.h"
#include "src/model/conference.h"
#include <QDebug>
#include <QHash>

Conference* ConferenceList::addConference(Core& core, int conferenceNum, const ConferenceId& conferenceId, const QString& name,
                           bool isAvConference, const QString& selfName, FriendList& friendList)
{
    auto checker = conferenceList.find(conferenceId);
    if (checker != conferenceList.end()) {
        qWarning() << "addConference: conferenceId already taken";
    }

    Conference* newConference =
        new Conference(conferenceNum, conferenceId, name, isAvConference, selfName, core, core, friendList);
    conferenceList[conferenceId] = newConference;
    id2key[conferenceNum] = conferenceId;
    return newConference;
}

Conference* ConferenceList::findConference(const ConferenceId& conferenceId)
{
    auto g_it = conferenceList.find(conferenceId);
    if (g_it != conferenceList.end())
        return *g_it;

    return nullptr;
}

const ConferenceId& ConferenceList::id2Key(uint32_t conferenceNum)
{
    return id2key[conferenceNum];
}

void ConferenceList::removeConference(const ConferenceId& conferenceId, bool /*fake*/)
{
    auto g_it = conferenceList.find(conferenceId);
    if (g_it != conferenceList.end()) {
        conferenceList.erase(g_it);
    }
}

QList<Conference*> ConferenceList::getAllConferences()
{
    QList<Conference*> res;

    for (auto it : conferenceList)
        res.append(it);

    return res;
}

void ConferenceList::clear()
{
    for (auto conferenceptr : conferenceList)
        delete conferenceptr;
    conferenceList.clear();
}
