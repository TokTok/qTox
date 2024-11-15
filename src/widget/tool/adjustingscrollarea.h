/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QScrollArea>

class AdjustingScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit AdjustingScrollArea(QWidget* parent = nullptr);
    virtual ~AdjustingScrollArea() = default;

protected:
    void resizeEvent(QResizeEvent* ev) override;
    QSize sizeHint() const final;
};
