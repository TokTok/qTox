/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019-2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/extension.h"

#include <QLabel>

class ExtensionStatus : public QLabel
{
    Q_OBJECT
public:
    ExtensionStatus(QWidget* parent = nullptr);

public slots:
    void onExtensionSetUpdate(ExtensionSet extensionSet);
};
