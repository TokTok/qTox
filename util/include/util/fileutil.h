/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#pragma once

template <typename T>
class QList;
class QString;

namespace FileUtil {
QList<QString> tail(const QString& filename, int count);
} // namespace FileUtil
