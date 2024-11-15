/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QScrollArea>

class QResizeEvent;
class QShowEvent;

class VerticalOnlyScroller : public QScrollArea
{
    Q_OBJECT
public:
    explicit VerticalOnlyScroller(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event) final;
    void showEvent(QShowEvent* event) final;
};
