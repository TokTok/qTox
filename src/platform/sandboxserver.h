/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#pragma once

#include "sandbox.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wheader-hygiene"
#include "rep_sandbox_source.h"
#pragma GCC diagnostic pop

#include <QObject>

class SandboxServer : public SandboxSimpleSource, public Sandbox
{
    Q_OBJECT

public:
    SandboxServer(QObject* parent = nullptr);
    ~SandboxServer() override;

    bool isReady() const override;
    QPixmap loadImage(const QByteArray& data) override;
};
