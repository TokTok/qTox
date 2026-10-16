/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#pragma once

#include <QByteArray>
#include <QImage>

namespace UntrustedImage {

/** @brief Decodes image data from a contact (avatars, received files).
 *
 * @return The decoded image, or a null image if @p data is not accepted.
 */
QImage decode(const QByteArray& data);

} // namespace UntrustedImage
