/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "text.h"

#include <QDateTime>
#include <QTextDocument>

class QTextDocument;
class DocumentCache;
class Settings;
class Style;

class Timestamp : public Text
{
    Q_OBJECT
public:
    Timestamp(const QDateTime& time_, const QString& format, const QFont& font,
              DocumentCache& documentCache, Settings& settings, Style& style);
    QDateTime getTime();

protected:
    QSizeF idealSize() override;

private:
    QDateTime time;
};
