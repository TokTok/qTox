/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "src/core/chatid.h"

#include <QObject>
#include <QString>

class ToxPk;
class Chat : public QObject
{
    Q_OBJECT
public:
    ~Chat() override;

    virtual void setName(const QString& name) = 0;
    virtual QString getDisplayedName() const = 0;
    virtual QString getDisplayedName(const ToxPk& contact) const = 0;
    virtual uint32_t getId() const = 0;
    virtual const ChatId& getPersistentId() const = 0;
    virtual void setEventFlag(bool flag) = 0;
    virtual bool getEventFlag() const = 0;

signals:
    void displayedNameChanged(const QString& newName);
};
