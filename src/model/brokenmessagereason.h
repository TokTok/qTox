/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

// NOTE: Numbers are important here as this is cast to an int and persisted in the DB
enum class BrokenMessageReason : int
{
    unknown = 0,
    unsupportedExtensions = 1,
};
