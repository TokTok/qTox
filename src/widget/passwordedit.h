/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QAction>
#include <QLineEdit>

class PasswordEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PasswordEdit(QWidget* parent);
    ~PasswordEdit();

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

private:
    class EventHandler : QObject
    {
    public:
        QVector<QAction*> actions;

        EventHandler();
        ~EventHandler();
        void updateActions();
        bool eventFilter(QObject* obj, QEvent* event);
    };

    void registerHandler();
    void unregisterHandler();

private:
    QAction* action;

    static EventHandler* eventHandler;
};
