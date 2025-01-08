/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#pragma once

#include "src/chatlog/imageloader.h"

class QPixmap;
class QByteArray;

class Sandbox : public ImageLoader
{
public:
    ~Sandbox() override;

    virtual bool isReady() const = 0;
};
