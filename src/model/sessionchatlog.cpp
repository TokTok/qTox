/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "sessionchatlog.h"
#include "src/conferencelist.h"
#include "src/friendlist.h"
#include "src/model/conference.h"

#include <QDebug>
#include <QtGlobal>

namespace {

constexpr QDate invalidDate;

/**
 * This function allows us to compare between a Message and QDateTime.
 */
QDate toDate(const std::pair<const ChatLogIdx, ChatLogItem>& item)
{
    return item.second.getContentType() == ChatLogItem::ContentType::message
               ? item.second.getContentAsMessage().message.timestamp.date()
               : invalidDate;
}

/**
 * @brief The search types all can be represented as some regular expression. This function
 *   takes the input phrase and filter and generates the appropriate regular expression
 * @return Regular expression which finds the input
 */
QRegularExpression getRegexpForPhrase(const QString& phrase, FilterSearch filter)
{
    constexpr auto regexFlags = QRegularExpression::UseUnicodePropertiesOption;
    constexpr auto caseInsensitiveFlags = QRegularExpression::CaseInsensitiveOption;

    switch (filter) {
    case FilterSearch::Register:
        return QRegularExpression(QRegularExpression::escape(phrase), regexFlags);
    case FilterSearch::WordsOnly:
        return QRegularExpression(SearchExtraFunctions::generateFilterWordsOnly(phrase),
                                  caseInsensitiveFlags);
    case FilterSearch::RegisterAndWordsOnly:
        return QRegularExpression(SearchExtraFunctions::generateFilterWordsOnly(phrase), regexFlags);
    case FilterSearch::RegisterAndRegular:
        return QRegularExpression(phrase, regexFlags);
    case FilterSearch::Regular:
        return QRegularExpression(phrase, caseInsensitiveFlags);
    default:
        return QRegularExpression(QRegularExpression::escape(phrase), caseInsensitiveFlags);
    }
}

/**
 * @return True if the given status indicates no future updates will come in
 */
bool toxFileIsComplete(ToxFile::FileStatus status)
{
    switch (status) {
    case ToxFile::INITIALIZING:
    case ToxFile::PAUSED:
    case ToxFile::TRANSMITTING:
        return false;
    case ToxFile::BROKEN:
    case ToxFile::CANCELED:
    case ToxFile::FINISHED:
    default:
        return true;
    }
}

std::map<ChatLogIdx, ChatLogItem>::const_iterator
firstItemAfterDate(QDate date, const std::map<ChatLogIdx, ChatLogItem>& items)
{
    return std::lower_bound(items.begin(), items.end(), date.startOfDay(),
                            [](const std::pair<const ChatLogIdx, ChatLogItem>& a,
                               const QDateTime& b) { return toDate(a) < b.date(); });
}

QString resolveToxPk(FriendList& friendList, ConferenceList& conferenceList, const ToxPk& pk)
{
    Friend* f = friendList.findFriend(pk);
    if (f) {
        return f->getDisplayedName();
    }

    for (Conference* it : conferenceList.getAllConferences()) {
        QString res = it->resolveToxPk(pk);
        if (!res.isEmpty()) {
            return res;
        }
    }

    return pk.toString();
}
} // namespace

SessionChatLog::SessionChatLog(const ICoreIdHandler& coreIdHandler_, FriendList& friendList_,
                               ConferenceList& conferenceList_)
    : coreIdHandler(coreIdHandler_)
    , friendList{friendList_}
    , conferenceList{conferenceList_}
{
}

/**
 * @brief Alternate constructor that allows for an initial index to be set
 */
SessionChatLog::SessionChatLog(ChatLogIdx initialIdx, const ICoreIdHandler& coreIdHandler_,
                               FriendList& friendList_, ConferenceList& conferenceList_)
    : coreIdHandler(coreIdHandler_)
    , nextIdx(initialIdx)
    , friendList{friendList_}
    , conferenceList{conferenceList_}
{
}

SessionChatLog::~SessionChatLog() = default;

QString SessionChatLog::resolveSenderNameFromSender(const ToxPk& sender)
{
    const bool isSelf = sender == coreIdHandler.getSelfPublicKey();
    const QString myNickName =
        coreIdHandler.getUsername().isEmpty() ? sender.toString() : coreIdHandler.getUsername();

    return isSelf ? myNickName : resolveToxPk(friendList, conferenceList, sender);
}

const ChatLogItem& SessionChatLog::at(ChatLogIdx idx) const
{
    auto item = items.find(idx);
    if (item == items.end()) {
        std::terminate();
    }

    return item->second;
}

SearchResult SessionChatLog::searchForward(SearchPos startPos, const QString& phrase,
                                           const ParameterSearch& parameter) const
{
    if (startPos.logIdx >= getNextIdx()) {
        SearchResult res;
        res.found = false;
        return res;
    }

    auto currentPos = startPos;

    auto regexp = getRegexpForPhrase(phrase, parameter.filter);

    for (auto it = items.find(currentPos.logIdx); it != items.end(); ++it) {
        const auto& key = it->first;
        const auto& item = it->second;

        if (item.getContentType() != ChatLogItem::ContentType::message) {
            continue;
        }

        const auto& content = item.getContentAsMessage();

        auto match = regexp.globalMatch(content.message.content, 0);

        auto numMatches = 0;
        QRegularExpressionMatch lastMatch;
        while (match.isValid() && numMatches <= static_cast<int>(currentPos.numMatches)
               && match.hasNext()) {
            lastMatch = match.next();
            numMatches++;
        }

        if (numMatches > static_cast<int>(currentPos.numMatches)) {
            SearchResult res;
            res.found = true;
            res.pos.logIdx = key;
            res.pos.numMatches = numMatches;
            res.start = lastMatch.capturedStart();
            res.len = lastMatch.capturedLength();
            return res;
        }

        // After the first iteration we force this to 0 to search the whole
        // message
        currentPos.numMatches = 0;
    }

    // We should have returned from the above loop if we had found anything
    SearchResult ret;
    ret.found = false;
    return ret;
}

SearchResult SessionChatLog::searchBackward(SearchPos startPos, const QString& phrase,
                                            const ParameterSearch& parameter) const
{
    auto currentPos = startPos;
    auto regexp = getRegexpForPhrase(phrase, parameter.filter);
    auto startIt = items.find(currentPos.logIdx);

    // If we don't have it we'll start at the end
    if (startIt == items.end()) {
        if (items.empty()) {
            SearchResult ret;
            ret.found = false;
            return ret;
        }
        startIt = std::prev(items.end());
        startPos.numMatches = 0;
    }

    // Off by 1 due to reverse_iterator api
    auto rStartIt = std::reverse_iterator<decltype(startIt)>(std::next(startIt));
    auto rEnd = std::reverse_iterator<decltype(startIt)>(items.begin());

    for (auto it = rStartIt; it != rEnd; ++it) {
        const auto& key = it->first;
        const auto& item = it->second;

        if (item.getContentType() != ChatLogItem::ContentType::message) {
            continue;
        }

        const auto& content = item.getContentAsMessage();
        auto match = regexp.globalMatch(content.message.content, 0);

        auto numMatchesBeforePos = 0;
        QRegularExpressionMatch lastMatch;
        while (match.isValid() && match.hasNext()) {
            auto currentMatch = match.next();
            if (currentPos.numMatches == 0
                || static_cast<int>(currentPos.numMatches) > numMatchesBeforePos) {
                lastMatch = currentMatch;
                numMatchesBeforePos++;
            }
        }

        if ((numMatchesBeforePos < static_cast<int>(currentPos.numMatches) || currentPos.numMatches == 0)
            && numMatchesBeforePos > 0) {
            SearchResult res;
            res.found = true;
            res.pos.logIdx = key;
            res.pos.numMatches = numMatchesBeforePos;
            res.start = lastMatch.capturedStart();
            res.len = lastMatch.capturedLength();
            return res;
        }

        // After the first iteration we force this to 0 to search the whole
        // message
        currentPos.numMatches = 0;
    }

    // We should have returned from the above loop if we had found anything
    SearchResult ret;
    ret.found = false;
    return ret;
}

ChatLogIdx SessionChatLog::getFirstIdx() const
{
    if (items.empty()) {
        return nextIdx;
    }

    return items.begin()->first;
}

ChatLogIdx SessionChatLog::getNextIdx() const
{
    return nextIdx;
}

std::vector<IChatLog::DateChatLogIdxPair> SessionChatLog::getDateIdxs(const QDate& startDate,
                                                                      size_t maxDates) const
{
    std::vector<DateChatLogIdxPair> ret;
    auto dateIt = startDate;

    while (true) {
        auto it = firstItemAfterDate(dateIt, items);

        if (it == items.end()) {
            break;
        }

        ret.emplace_back(DateChatLogIdxPair{dateIt, it->first});

        dateIt = dateIt.addDays(1);
        if (startDate.daysTo(dateIt) > static_cast<long>(maxDates) && maxDates != 0) {
            break;
        }
    }

    return ret;
}

void SessionChatLog::addSystemMessage(const SystemMessage& message)
{
    auto messageIdx = nextIdx++;

    items.emplace(messageIdx, ChatLogItem(message));

    emit itemUpdated(messageIdx);
}

void SessionChatLog::insertCompleteMessageAtIdx(ChatLogIdx idx, const ToxPk& sender,
                                                QString senderName, const ChatLogMessage& message)
{
    auto item = ChatLogItem(sender, senderName, message);

    assert(message.state == MessageState::complete);

    items.emplace(idx, std::move(item));
}

void SessionChatLog::insertIncompleteMessageAtIdx(ChatLogIdx idx, const ToxPk& sender,
                                                  QString senderName, const ChatLogMessage& message,
                                                  DispatchedMessageId dispatchId)
{
    auto item = ChatLogItem(sender, senderName, message);

    assert(message.state == MessageState::pending);

    items.emplace(idx, std::move(item));
    outgoingMessages.insert(dispatchId, idx);
}

void SessionChatLog::insertBrokenMessageAtIdx(ChatLogIdx idx, const ToxPk& sender,
                                              QString senderName, const ChatLogMessage& message)
{
    auto item = ChatLogItem(sender, senderName, message);

    assert(message.state == MessageState::broken);

    items.emplace(idx, std::move(item));
}

void SessionChatLog::insertFileAtIdx(ChatLogIdx idx, const ToxPk& sender, QString senderName,
                                     const ChatLogFile& file)
{
    auto item = ChatLogItem(sender, senderName, file);

    items.emplace(idx, std::move(item));
}

void SessionChatLog::insertSystemMessageAtIdx(ChatLogIdx idx, SystemMessage message)
{
    auto item = ChatLogItem(std::move(message));

    items.emplace(idx, std::move(item));
}

/**
 * @brief Inserts message data into the chat log buffer
 * @note Owner of SessionChatLog is in charge of attaching this to the appropriate IMessageDispatcher
 */
void SessionChatLog::onMessageReceived(const ToxPk& sender, const Message& message)
{
    auto messageIdx = nextIdx++;

    ChatLogMessage chatLogMessage;
    chatLogMessage.state = MessageState::complete;
    chatLogMessage.message = message;
    items.emplace(messageIdx, ChatLogItem(sender, resolveSenderNameFromSender(sender), chatLogMessage));

    emit itemUpdated(messageIdx);
}

/**
 * @brief Inserts message data into the chat log buffer
 * @note Owner of SessionChatLog is in charge of attaching this to the appropriate IMessageDispatcher
 */
void SessionChatLog::onMessageSent(DispatchedMessageId id, const Message& message)
{
    auto messageIdx = nextIdx++;

    ChatLogMessage chatLogMessage;
    chatLogMessage.state = MessageState::pending;
    chatLogMessage.message = message;
    const ToxPk selfPk = coreIdHandler.getSelfPublicKey();
    const QString selfName = resolveSenderNameFromSender(selfPk);
    items.emplace(messageIdx, ChatLogItem(selfPk, selfName, chatLogMessage));

    outgoingMessages.insert(id, messageIdx);

    emit itemUpdated(messageIdx);
}

/**
 * @brief Marks the associated message as complete and notifies any listeners
 * @note Owner of SessionChatLog is in charge of attaching this to the appropriate IMessageDispatcher
 */
void SessionChatLog::onMessageComplete(DispatchedMessageId id)
{
    auto chatLogIdxIt = outgoingMessages.find(id);

    if (chatLogIdxIt == outgoingMessages.end()) {
        qWarning() << "Failed to find outgoing message";
        return;
    }

    const auto& chatLogIdx = *chatLogIdxIt;
    auto messageIt = items.find(chatLogIdx);

    if (messageIt == items.end()) {
        qWarning() << "Failed to look up message in chat log";
        return;
    }

    messageIt->second.getContentAsMessage().state = MessageState::complete;

    emit itemUpdated(messageIt->first);
}

void SessionChatLog::onMessageBroken(DispatchedMessageId id, BrokenMessageReason reason)
{
    std::ignore = reason;
    auto chatLogIdxIt = outgoingMessages.find(id);

    if (chatLogIdxIt == outgoingMessages.end()) {
        qWarning() << "Failed to find outgoing message";
        return;
    }

    const auto& chatLogIdx = *chatLogIdxIt;
    auto messageIt = items.find(chatLogIdx);

    if (messageIt == items.end()) {
        qWarning() << "Failed to look up message in chat log";
        return;
    }

    // NOTE: Reason for broken message not currently shown in UI, but it could be
    messageIt->second.getContentAsMessage().state = MessageState::broken;

    emit itemUpdated(messageIt->first);
}

/**
 * @brief Updates file state in the chat log
 * @note The files need to be pre-filtered for the current chat since we do no validation
 * @note This should be attached to any CoreFile signal that fits the signature
 */
void SessionChatLog::onFileUpdated(const ToxPk& sender, const ToxFile& file)
{
    auto fileIt =
        std::find_if(currentFileTransfers.begin(), currentFileTransfers.end(),
                     [&](const CurrentFileTransfer& transfer) { return transfer.file == file; });

    ChatLogIdx messageIdx;
    if (fileIt == currentFileTransfers.end() && file.status == ToxFile::INITIALIZING) {
        assert(file.status == ToxFile::INITIALIZING);
        CurrentFileTransfer currentTransfer;
        currentTransfer.file = file;
        currentTransfer.idx = nextIdx++;
        currentFileTransfers.push_back(currentTransfer);

        const auto chatLogFile = ChatLogFile{QDateTime::currentDateTime(), file};
        items.emplace(currentTransfer.idx,
                      ChatLogItem(sender, resolveSenderNameFromSender(sender), chatLogFile));
        messageIdx = currentTransfer.idx;
    } else if (fileIt != currentFileTransfers.end()) {
        messageIdx = fileIt->idx;
        fileIt->file = file;

        items.at(messageIdx).getContentAsFile().file = file;
    } else {
        // This may be a file unbroken message that we don't handle ATM
        return;
    }

    if (toxFileIsComplete(file.status)) {
        currentFileTransfers.erase(fileIt);
    }

    emit itemUpdated(messageIdx);
}

void SessionChatLog::onFileTransferRemotePausedUnpaused(const ToxPk& sender, const ToxFile& file,
                                                        bool /*paused*/)
{
    onFileUpdated(sender, file);
}

void SessionChatLog::onFileTransferBrokenUnbroken(const ToxPk& sender, const ToxFile& file,
                                                  bool /*broken*/)
{
    onFileUpdated(sender, file);
}
