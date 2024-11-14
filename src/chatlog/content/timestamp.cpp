/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "timestamp.h"

Timestamp::Timestamp(const QDateTime& time_, const QString& format,
    const QFont& font, DocumentCache& documentCache_, Settings& settings_,
    Style& style_)
    : Text(documentCache_, settings_, style_, time_.toString(format), font, false,
        time_.toString(format))
{
    time = time_;
}

QDateTime Timestamp::getTime()
{
    return time;
}

QSizeF Timestamp::idealSize()
{
    if (doc) {
        return QSizeF(qMin(doc->idealWidth(), width), doc->size().height());
    }
    return size;
}
