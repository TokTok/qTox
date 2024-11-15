/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */


#pragma once

#include <memory>

class IAudioControl;
class IAudioSettings;
class Audio
{
public:
    static std::unique_ptr<IAudioControl> makeAudio(IAudioSettings& settings);
};
