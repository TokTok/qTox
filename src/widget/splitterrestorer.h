/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

class QSize;
class QSplitter;
class QByteArray;

class SplitterRestorer
{
public:
    explicit SplitterRestorer(QSplitter* splitter_);
    void restore(const QByteArray& state, const QSize& windowSize);

private:
    QSplitter* splitter;
};
