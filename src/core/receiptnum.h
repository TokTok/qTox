/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "util/strongtype.h"

#include <QMetaType>

#include <cstdint>

using ReceiptNum = NamedType<uint32_t, struct ReceiptNumTag, Orderable>;
Q_DECLARE_METATYPE(ReceiptNum)

using ExtendedReceiptNum = NamedType<uint32_t, struct ExtendedReceiptNumTag, Orderable>;
Q_DECLARE_METATYPE(ExtendedReceiptNum)
