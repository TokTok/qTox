/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "audio/audio.h"
#include "audio/iaudiosettings.h"
#include "backend/openal.h"

#include <memory>

/**
 * @brief Select the audio backend
 * @param settings Audio settings to use
 * @return Audio backend selection based on settings
 */
std::unique_ptr<IAudioControl> Audio::makeAudio(IAudioSettings& settings)
{
    Q_INIT_RESOURCE(audio_res);
    return std::unique_ptr<IAudioControl>(new OpenAL(settings));
}
