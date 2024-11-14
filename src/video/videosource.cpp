/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2016-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "videosource.h"

/**
 * @class VideoSource
 * @brief An abstract source of video frames
 *
 * When it has at least one subscriber the source will emit new video frames.
 * Subscribing is recursive, multiple users can subscribe to the same VideoSource.
 */

// Initialize sourceIDs to 0
VideoSource::AtomicIDType VideoSource::sourceIDs{0};
