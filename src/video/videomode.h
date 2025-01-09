/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QRect>

#include <cstdint>

struct VideoMode
{
    int width, height;
    int x, y;
    float fps = -1.0f;
    uint32_t pixel_format = 0;

    explicit constexpr VideoMode(int width_ = 0, int height_ = 0, int x_ = 0, int y_ = 0,
                                 float fps_ = -1.0f)
        : width(width_)
        , height(height_)
        , x(x_)
        , y(y_)
        , fps(fps_)
    {
    }

    explicit VideoMode(QRect rect);

    QRect toRect() const;

    bool isUnspecified() const;
    bool operator==(const VideoMode& other) const;
    uint32_t norm(const VideoMode& other) const;
    uint32_t tolerance() const;
};
