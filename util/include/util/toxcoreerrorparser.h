/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QDebug>

#include <tox/tox.h>
#include <tox/toxav.h>

/**
 * @brief Parse and log toxcore error enums.
 * @param error Error to handle.
 * @return True if no error, false otherwise.
 */
#define PARSE_ERR(err) ToxcoreErrorParser::parseErr(err, __LINE__)

namespace ToxcoreErrorParser {
    bool parseErr(Tox_Err_Conference_Title error, int line);
    bool parseErr(Tox_Err_Friend_Send_Message error, int line);
    bool parseErr(Tox_Err_Conference_Send_Message error, int line);
    bool parseErr(Tox_Err_Conference_Peer_Query error, int line);
    bool parseErr(Tox_Err_Conference_Join error, int line);
    bool parseErr(Tox_Err_Conference_Get_Type error, int line);
    bool parseErr(Tox_Err_Conference_Invite error, int line);
    bool parseErr(Tox_Err_Conference_New error, int line);
    bool parseErr(Tox_Err_Friend_By_Public_Key error, int line);
    bool parseErr(Tox_Err_Bootstrap error, int line);
    bool parseErr(Tox_Err_Friend_Add error, int line);
    bool parseErr(Tox_Err_Friend_Delete error, int line);
    bool parseErr(Tox_Err_Set_Info error, int line);
    bool parseErr(Tox_Err_Friend_Query error, int line);
    bool parseErr(Tox_Err_Friend_Get_Public_Key error, int line);
    bool parseErr(Tox_Err_Friend_Get_Last_Online error, int line);
    bool parseErr(Tox_Err_Set_Typing error, int line);
    bool parseErr(Tox_Err_Conference_Delete error, int line);
    bool parseErr(Tox_Err_Get_Port error, int line);
    bool parseErr(Tox_Err_File_Control error, int line);
    bool parseErr(Tox_Err_File_Get error, int line);
    bool parseErr(Tox_Err_File_Send error, int line);
    bool parseErr(Tox_Err_File_Send_Chunk error, int line);
    bool parseErr(Toxav_Err_Bit_Rate_Set error, int line);
    bool parseErr(Toxav_Err_Call_Control error, int line);
    bool parseErr(Toxav_Err_Call error, int line);
    bool parseErr(Tox_Err_Options_New error, int line);
} // namespace ToxcoreErrorParser
