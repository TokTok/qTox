/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include <QDebug>

#include <tox/tox.h>
#include <tox/tox_events.h>
#include <tox/toxav.h>

/**
 * @brief Parse and log toxcore error enums.
 * @param error Error to handle.
 * @return True if no error, false otherwise.
 */
#define PARSE_ERR(err) \
    ToxcoreErrorParser::parseErr(err, QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC)

namespace ToxcoreErrorParser {
bool parseErr(Tox_Err_Conference_Title error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Friend_Send_Message error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Conference_Send_Message error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Conference_Peer_Query error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Conference_Join error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Conference_Get_Type error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Conference_Invite error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Conference_New error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Friend_By_Public_Key error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Bootstrap error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Friend_Add error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Friend_Delete error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Set_Info error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Friend_Query error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Friend_Get_Public_Key error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Friend_Get_Last_Online error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Set_Typing error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Conference_Delete error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Get_Port error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_File_Control error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_File_Get error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_File_Send error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_File_Send_Chunk error, const char* file, int line, const char* func);
bool parseErr(Toxav_Err_Bit_Rate_Set error, const char* file, int line, const char* func);
bool parseErr(Toxav_Err_Call_Control error, const char* file, int line, const char* func);
bool parseErr(Toxav_Err_Call error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Options_New error, const char* file, int line, const char* func);
bool parseErr(Tox_Err_Events_Iterate error, const char* file, int line, const char* func);
} // namespace ToxcoreErrorParser
