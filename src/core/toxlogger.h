/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "src/core/idebugsettings.h"

struct Tox_Options;

namespace ToxLogger {

void setMinLevel(IDebugSettings::ToxcoreLogLevel level);
void setLogCallback(Tox_Options* options);

} // namespace ToxLogger
