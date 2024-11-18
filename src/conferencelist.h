/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/conferenceid.h"

class Core;
template <class A, class B>
class QHash;
template <class T>
class QList;
class Conference;
class QString;
class FriendList;

class ConferenceList
{
public:
    Conference* addConference(Core& core, int conferenceNum, const ConferenceId& persistentConferenceId, const QString& name,
                    bool isAvConference, const QString& selfName, FriendList& friendList);
    Conference* findConference(const ConferenceId& conferenceId);
    const ConferenceId& id2Key(uint32_t conferenceNum);
    void removeConference(const ConferenceId& conferenceId, bool fake = false);
    QList<Conference*> getAllConferences();
    void clear();

private:
    QHash<const ConferenceId, Conference*> conferenceList;
    QHash<uint32_t, ConferenceId> id2key;
};
