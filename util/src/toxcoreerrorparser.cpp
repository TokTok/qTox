/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "util/toxcoreerrorparser.h"

#include <QDebug>

#include <tox/tox.h>

#define qCriticalFrom(file, line, func) QMessageLogger(file, line, func).critical()

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Title error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_TITLE_OK:
        return true;

    case TOX_ERR_CONFERENCE_TITLE_CONFERENCE_NOT_FOUND:
        qCriticalFrom(file, line, func) << ": Conference title not found";
        return false;

    case TOX_ERR_CONFERENCE_TITLE_INVALID_LENGTH:
        qCriticalFrom(file, line, func) << ": Invalid conference title length";
        return false;

    case TOX_ERR_CONFERENCE_TITLE_FAIL_SEND:
        qCriticalFrom(file, line, func) << ": Failed to send title packet";
        return false;
    }
    qCriticalFrom(file, line, func) << ": Unknown Tox_Err_Conference_Title error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Send_Message error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FRIEND_SEND_MESSAGE_OK:
        return true;

    case TOX_ERR_FRIEND_SEND_MESSAGE_NULL:
        qCriticalFrom(file, line, func) << "Send friend message passed an unexpected null argument";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << "Send friend message could not find friend";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED:
        qCriticalFrom(file, line, func) << "Send friend message: friend is offline";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ:
        qCriticalFrom(file, line, func) << "Failed to allocate more message queue";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG:
        qCriticalFrom(file, line, func) << "Attempted to send message that's too long";
        return false;

    case TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY:
        qCriticalFrom(file, line, func) << "Attempted to send an empty message";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown friend send message error:" << static_cast<int>(error);
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Send_Message error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_SEND_MESSAGE_OK:
        return true;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_CONFERENCE_NOT_FOUND:
        qCriticalFrom(file, line, func) << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_FAIL_SEND:
        qCriticalFrom(file, line, func) << "Conference message failed to send";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_NO_CONNECTION:
        qCriticalFrom(file, line, func) << "No connection";
        return false;

    case TOX_ERR_CONFERENCE_SEND_MESSAGE_TOO_LONG:
        qCriticalFrom(file, line, func) << "Message too long";
        return false;
    }
    qCriticalFrom(file, line, func)
        << "Unknown Tox_Err_Conference_Send_Message  error:" << static_cast<int>(error);
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Peer_Query error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_PEER_QUERY_OK:
        return true;

    case TOX_ERR_CONFERENCE_PEER_QUERY_CONFERENCE_NOT_FOUND:
        qCriticalFrom(file, line, func) << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_PEER_QUERY_NO_CONNECTION:
        qCriticalFrom(file, line, func) << "No connection";
        return false;

    case TOX_ERR_CONFERENCE_PEER_QUERY_PEER_NOT_FOUND:
        qCriticalFrom(file, line, func) << "Peer not found";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Conference_Peer_Query error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Join error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_JOIN_OK:
        return true;

    case TOX_ERR_CONFERENCE_JOIN_DUPLICATE:
        qCriticalFrom(file, line, func) << "Conference duplicate";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_FAIL_SEND:
        qCriticalFrom(file, line, func) << "Conference join failed to send";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << "Friend not found";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_INIT_FAIL:
        qCriticalFrom(file, line, func) << "Init fail";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_INVALID_LENGTH:
        qCriticalFrom(file, line, func) << "Invalid length";
        return false;

    case TOX_ERR_CONFERENCE_JOIN_WRONG_TYPE:
        qCriticalFrom(file, line, func) << "Wrong conference type";
        return false;

#if TOX_VERSION_IS_API_COMPATIBLE(0, 2, 20)
    case TOX_ERR_CONFERENCE_JOIN_NULL:
        qCriticalFrom(file, line, func) << "Null argument";
        return false;
#endif
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Conference_Join error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Get_Type error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_GET_TYPE_OK:
        return true;

    case TOX_ERR_CONFERENCE_GET_TYPE_CONFERENCE_NOT_FOUND:
        qCriticalFrom(file, line, func) << "Conference not found";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Conference_Get_Type error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Invite error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_INVITE_OK:
        return true;

    case TOX_ERR_CONFERENCE_INVITE_CONFERENCE_NOT_FOUND:
        qCriticalFrom(file, line, func) << "Conference not found";
        return false;

    case TOX_ERR_CONFERENCE_INVITE_FAIL_SEND:
        qCriticalFrom(file, line, func) << "Conference invite failed to send";
        return false;

    case TOX_ERR_CONFERENCE_INVITE_NO_CONNECTION:
        qCriticalFrom(file, line, func)
            << "Cannot invite to conference that we're not connected to";
        return false;
    }
    qWarning() << "Unknown Tox_Err_Conference_Invite error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_New error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_NEW_OK:
        return true;

    case TOX_ERR_CONFERENCE_NEW_INIT:
        qCriticalFrom(file, line, func) << "The conference instance failed to initialize";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Conference_New error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_By_Public_Key error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_OK:
        return true;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NULL:
        qCriticalFrom(file, line, func) << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_BY_PUBLIC_KEY_NOT_FOUND:
        // we use this as a check for friendship, so this can be an expected result
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Friend_By_Public_Key error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Bootstrap error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_BOOTSTRAP_OK:
        return true;

    case TOX_ERR_BOOTSTRAP_NULL:
        qCriticalFrom(file, line, func) << "null argument when not expected";
        return false;

    case TOX_ERR_BOOTSTRAP_BAD_HOST:
        qCriticalFrom(file, line, func) << "Could not resolve hostname, or invalid IP address";
        return false;

    case TOX_ERR_BOOTSTRAP_BAD_PORT:
        qCriticalFrom(file, line, func) << "out of range port";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_bootstrap error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Add error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_FRIEND_ADD_OK:
        return true;

    case TOX_ERR_FRIEND_ADD_NULL:
        qCriticalFrom(file, line, func) << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_ADD_TOO_LONG:
        qCriticalFrom(file, line, func) << "The length of the friend request message exceeded";
        return false;

    case TOX_ERR_FRIEND_ADD_NO_MESSAGE:
        qCriticalFrom(file, line, func) << "The friend request message was empty.";
        return false;

    case TOX_ERR_FRIEND_ADD_OWN_KEY:
        qCriticalFrom(file, line, func) << "The friend address belongs to the sending client.";
        return false;

    case TOX_ERR_FRIEND_ADD_ALREADY_SENT:
        qCriticalFrom(file, line, func)
            << "The address belongs to a friend that is already on the friend list.";
        return false;

    case TOX_ERR_FRIEND_ADD_BAD_CHECKSUM:
        qCriticalFrom(file, line, func) << "The friend address checksum failed.";
        return false;

    case TOX_ERR_FRIEND_ADD_SET_NEW_NOSPAM:
        qCriticalFrom(file, line, func)
            << "The address belongs to a friend that is already on the friend list.";
        return false;

    case TOX_ERR_FRIEND_ADD_MALLOC:
        qCriticalFrom(file, line, func)
            << "A memory allocation failed when trying to increase the friend list size.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Friend_Add error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Delete error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FRIEND_DELETE_OK:
        return true;

    case TOX_ERR_FRIEND_DELETE_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << "There is no friend with the given friend number";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Friend_Delete error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Set_Info error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_SET_INFO_OK:
        return true;

    case TOX_ERR_SET_INFO_NULL:
        qCriticalFrom(file, line, func) << "null argument when not expected";
        return false;

    case TOX_ERR_SET_INFO_TOO_LONG:
        qCriticalFrom(file, line, func) << "Information length exceeded maximum permissible size.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Set_Info error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Query error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FRIEND_QUERY_OK:
        return true;

    case TOX_ERR_FRIEND_QUERY_NULL:
        qCriticalFrom(file, line, func) << "null argument when not expected";
        return false;

    case TOX_ERR_FRIEND_QUERY_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << "The friend_number did not designate a valid friend.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Friend_Query error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Get_Public_Key error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_OK:
        return true;

    case TOX_ERR_FRIEND_GET_PUBLIC_KEY_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << "There is no friend with the given friend number";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Friend_Get_Public_Key error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Friend_Get_Last_Online error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FRIEND_GET_LAST_ONLINE_OK:
        return true;

    case TOX_ERR_FRIEND_GET_LAST_ONLINE_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << "There is no friend with the given friend number";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Friend_Get_Last_Online error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Set_Typing error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_SET_TYPING_OK:
        return true;

    case TOX_ERR_SET_TYPING_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << "There is no friend with the given friend number";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Set_Typing error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Conference_Delete error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_CONFERENCE_DELETE_OK:
        return true;

    case TOX_ERR_CONFERENCE_DELETE_CONFERENCE_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << "The conference number passed did not designate a valid conference.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Conference_Delete error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Get_Port error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_GET_PORT_OK:
        return true;

    case TOX_ERR_GET_PORT_NOT_BOUND:
        qCriticalFrom(file, line, func) << "Tox instance was not bound to any port.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Get_Port error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Control error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FILE_CONTROL_OK:
        return true;

    case TOX_ERR_FILE_CONTROL_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_CONTROL_FRIEND_NOT_CONNECTED:
        qCriticalFrom(file, line, func)
            << ": This client is currently not connected to the friend.";
        return false;

    case TOX_ERR_FILE_CONTROL_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": No file transfer with the given file number was found for the given friend.";
        return false;

    case TOX_ERR_FILE_CONTROL_NOT_PAUSED:
        qCriticalFrom(file, line, func)
            << ": A RESUME control was sent, but the file transfer is running normally.";
        return false;

    case TOX_ERR_FILE_CONTROL_DENIED:
        qCriticalFrom(file, line, func)
            << ": A RESUME control was sent, but the file transfer was paused by the other party.";
        return false;

    case TOX_ERR_FILE_CONTROL_ALREADY_PAUSED:
        qCriticalFrom(file, line, func)
            << ": A PAUSE control was sent, but the file transfer was already paused.";
        return false;

    case TOX_ERR_FILE_CONTROL_SENDQ:
        qCriticalFrom(file, line, func) << ": Packet queue is full.";
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_File_Control error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Get error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_FILE_GET_OK:
        return true;

    case TOX_ERR_FILE_GET_NULL:
        qCriticalFrom(file, line, func)
            << ": One of the arguments to the function was NULL when it was not expected.";
        return false;

    case TOX_ERR_FILE_GET_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_GET_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": The friend_number passed did not designate a valid friend.";
        return false;
    }
    qCriticalFrom(file, line, func) << ": Unknown Tox_Err_Conference_Title error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Send error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_FILE_SEND_OK:
        return true;

    case TOX_ERR_FILE_SEND_NULL:
        qCriticalFrom(file, line, func)
            << ": One of the arguments to the function was NULL when it was not expected.";
        return false;

    case TOX_ERR_FILE_SEND_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_SEND_FRIEND_NOT_CONNECTED:
        qCriticalFrom(file, line, func)
            << ": This client is currently not connected to the friend.";
        return false;

    case TOX_ERR_FILE_SEND_NAME_TOO_LONG:
        qCriticalFrom(file, line, func)
            << ": Filename length exceeded TOX_MAX_FILENAME_LENGTH bytes.";
        return false;

    case TOX_ERR_FILE_SEND_TOO_MANY:
        qCriticalFrom(file, line, func) << ": Too many ongoing transfers.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_File_Send error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_File_Send_Chunk error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOX_ERR_FILE_SEND_CHUNK_OK:
        return true;

    case TOX_ERR_FILE_SEND_CHUNK_NULL:
        qCriticalFrom(file, line, func)
            << ": The length parameter was non-zero, but data was NULL.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_FRIEND_NOT_CONNECTED:
        qCriticalFrom(file, line, func)
            << ": This client is currently not connected to the friend.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": No file transfer with the given file number was found for the given friend.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_NOT_TRANSFERRING:
        qCriticalFrom(file, line, func)
            << ": File transfer was found but isn't in a transferring state.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_INVALID_LENGTH:
        qCriticalFrom(file, line, func) << ": Attempted to send more or less data than requested.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_SENDQ:
        qCriticalFrom(file, line, func) << ": Packet queue is full.";
        return false;

    case TOX_ERR_FILE_SEND_CHUNK_WRONG_POSITION:
        qCriticalFrom(file, line, func) << ": Position parameter was wrong.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_File_Send_Chunk error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Toxav_Err_Bit_Rate_Set error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOXAV_ERR_BIT_RATE_SET_OK:
        return true;

    case TOXAV_ERR_BIT_RATE_SET_SYNC:
        qCriticalFrom(file, line, func) << ": Synchronization error occurred.";
        return false;

    case TOXAV_ERR_BIT_RATE_SET_INVALID_BIT_RATE:
        qCriticalFrom(file, line, func)
            << ": The bit rate passed was not one of the supported values.";
        return false;

    case TOXAV_ERR_BIT_RATE_SET_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOXAV_ERR_BIT_RATE_SET_FRIEND_NOT_IN_CALL:
        qCriticalFrom(file, line, func)
            << ": This client is currently not in a call with the friend.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Toxav_Err_Bit_Rate_Set error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Toxav_Err_Call_Control error, const char* file, int line,
                                  const char* func)
{
    switch (error) {
    case TOXAV_ERR_CALL_CONTROL_OK:
        return true;

    case TOXAV_ERR_CALL_CONTROL_SYNC:
        qCriticalFrom(file, line, func) << ": Synchronization error occurred.";
        return false;

    case TOXAV_ERR_CALL_CONTROL_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func)
            << ": The friend_number passed did not designate a valid friend.";
        return false;

    case TOXAV_ERR_CALL_CONTROL_FRIEND_NOT_IN_CALL:
        qCriticalFrom(file, line, func)
            << ": This client is currently not in a call with the friend.";
        return false;

    case TOXAV_ERR_CALL_CONTROL_INVALID_TRANSITION:
        qCriticalFrom(file, line, func) << ": Call already paused or resumed.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Toxav_Err_Call_Control error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Toxav_Err_Call error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOXAV_ERR_CALL_OK:
        return true;

    case TOXAV_ERR_CALL_MALLOC:
        qCriticalFrom(file, line, func) << ": A resource allocation error occurred.";
        return false;

    case TOXAV_ERR_CALL_SYNC:
        qCriticalFrom(file, line, func) << ": Synchronization error occurred.";
        return false;

    case TOXAV_ERR_CALL_FRIEND_NOT_FOUND:
        qCriticalFrom(file, line, func) << ": The friend number did not designate a valid friend.";
        return false;

    case TOXAV_ERR_CALL_FRIEND_NOT_CONNECTED:
        qCriticalFrom(file, line, func) << ": The friend was valid, but not currently connected.";
        return false;

    case TOXAV_ERR_CALL_FRIEND_ALREADY_IN_CALL:
        qCriticalFrom(file, line, func)
            << ": Attempted to call a friend while already in a call with them.";
        return false;

    case TOXAV_ERR_CALL_INVALID_BIT_RATE:
        qCriticalFrom(file, line, func) << ": Audio or video bit rate is invalid.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Toxav_Err_Call error code:" << error;
    return false;
}

bool ToxcoreErrorParser::parseErr(Tox_Err_Options_New error, const char* file, int line, const char* func)
{
    switch (error) {
    case TOX_ERR_OPTIONS_NEW_OK:
        return true;

    case TOX_ERR_OPTIONS_NEW_MALLOC:
        qCriticalFrom(file, line, func) << ": Failed to allocate memory.";
        return false;
    }
    qCriticalFrom(file, line, func) << "Unknown Tox_Err_Options_New error code:" << error;
    return false;
}
