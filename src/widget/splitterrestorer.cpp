/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "src/widget/splitterrestorer.h"

#include <QSplitter>

/**
 * @class SplitterRestorer
 * @brief Restore splitter from saved state and reset to default
 */

namespace {
/**
 * @brief The width of the default splitter handles.
 * By default, this property contains a value that depends on the user's
 * platform and style preferences.
 */
int defaultWidth = 0;

/**
 * @brief Width of left splitter size in percents.
 */
const int leftWidthPercent = 33;
} // namespace

SplitterRestorer::SplitterRestorer(QSplitter* splitter_)
    : splitter{splitter_}
{
    if (defaultWidth == 0) {
        defaultWidth = QSplitter().handleWidth();
    }
}

/**
 * @brief Restore splitter from state. And reset in case of error.
 * Set the splitter to a reasonnable width by default and on first start
 * @param state State saved by QSplitter::saveState()
 * @param windowSize Widnow size (used to calculate splitter size)
 */
void SplitterRestorer::restore(const QByteArray& state, const QSize& windowSize)
{
    bool brokenSplitter = !splitter->restoreState(state) || splitter->orientation() != Qt::Horizontal
                          || splitter->handleWidth() > defaultWidth;

    if (splitter->count() == 2 && brokenSplitter) {
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(defaultWidth);
        splitter->resize(windowSize);
        QList<int> sizes = splitter->sizes();
        sizes[0] = splitter->width() * leftWidthPercent / 100;
        sizes[1] = splitter->width() - sizes[0];
        splitter->setSizes(sizes);
    }
}
