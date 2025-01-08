/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#pragma once

#include "sandbox.h"
#include <QObject>

#include <memory>

class QPixmap;
class QByteArray;

class SandboxClient : public QObject, public Sandbox
{
    Q_OBJECT

public:
    SandboxClient(QObject* parent);
    ~SandboxClient();

    bool isReady() const override;
    QPixmap loadImage(const QByteArray& data) override;

private:
    struct Data;
    std::unique_ptr<Data> d;
};
