/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "message.h"
#include "src/conferencelist.h"
#include "src/core/toxfile.h"
#include "src/core/toxpk.h"
#include "src/friendlist.h"
#include "src/model/chatlogitem.h"
#include "src/model/conference.h"
#include "src/model/friend.h"
#include "src/model/systemmessage.h"
#include "src/widget/searchtypes.h"
#include "util/strongtype.h"

#include <cassert>

using ChatLogIdx =
    NamedType<size_t, struct ChatLogIdxTag, Orderable, UnderlyingAddable, UnitlessSubtractable, Incrementable>;
Q_DECLARE_METATYPE(ChatLogIdx)

struct SearchPos
{
    // Index to the chat log item we want
    ChatLogIdx logIdx;
    // Number of matches we've had. This is always number of matches from the
    // start even if we're searching backwards.
    size_t numMatches{0};

    bool operator==(const SearchPos& other) const
    {
        return tie() == other.tie();
    }

    bool operator!=(const SearchPos& other) const
    {
        return tie() != other.tie();
    }

    bool operator<(const SearchPos& other) const
    {
        return tie() < other.tie();
    }

    std::tuple<ChatLogIdx, size_t> tie() const
    {
        return std::tie(logIdx, numMatches);
    }
};

struct SearchResult
{
    bool found{false};
    SearchPos pos;
    size_t start{0};
    size_t len{0};

    // This is unfortunately needed to shoehorn our API into the highlighting
    // API of above classes. They expect to re-search the same thing we did
    // for some reason
    QRegularExpression exp;
};

class IChatLog : public QObject
{
    Q_OBJECT
public:
    virtual ~IChatLog() = default;

    /**
     * @brief Returns reference to item at idx
     * @param[in] idx
     * @return Variant type referencing either a ToxFile or Message
     * @pre idx must be between currentFirstIdx() and currentLastIdx()
     */
    virtual const ChatLogItem& at(ChatLogIdx idx) const = 0;

    /**
     * @brief searches forwards through the chat log until phrase is found according to parameter
     * @param[in] startIdx inclusive start idx
     * @param[in] phrase phrase to find (may be modified by parameter)
     * @param[in] parameter search parameters
     */
    virtual SearchResult searchForward(SearchPos startIdx, const QString& phrase,
                                       const ParameterSearch& parameter) const = 0;

    /**
     * @brief searches backwards through the chat log until phrase is found according to parameter
     * @param[in] startIdx inclusive start idx
     * @param[in] phrase phrase to find (may be modified by parameter)
     * @param[in] parameter search parameters
     */
    virtual SearchResult searchBackward(SearchPos startIdx, const QString& phrase,
                                        const ParameterSearch& parameter) const = 0;

    /**
     * @brief The underlying chat log instance may not want to start at 0
     * @return Current first valid index to call at() with
     */
    virtual ChatLogIdx getFirstIdx() const = 0;

    /**
     * @return current last valid index to call at() with
     */
    virtual ChatLogIdx getNextIdx() const = 0;

    struct DateChatLogIdxPair
    {
        QDate date;
        ChatLogIdx idx;
    };

    /**
     * @brief Gets indexes for each new date starting at startDate
     * @param[in] startDate date to start searching from
     * @param[in] maxDates maximum number of dates to be returned
     */
    virtual std::vector<DateChatLogIdxPair> getDateIdxs(const QDate& startDate,
                                                        size_t maxDates) const = 0;

    /**
     * @brief Inserts a system message at the end of the chat
     * @param[in] message systemMessage to insert
     */
    virtual void addSystemMessage(const SystemMessage& message) = 0;

signals:
    void itemUpdated(ChatLogIdx idx);
};
