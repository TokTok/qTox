/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#include "passwordedit.h"

#include <QCoreApplication>

#include "src/platform/capslock.h"

std::unique_ptr<PasswordEdit::EventHandler> PasswordEdit::eventHandler{nullptr};

PasswordEdit::PasswordEdit(QWidget* parent)
    : QLineEdit(parent)
    , action(new QAction(this))
{
    setEchoMode(QLineEdit::Password);

#ifdef QTOX_PLATFORM_EXT
    action->setIcon(QIcon(":img/caps_lock.svg"));
    action->setToolTip(tr("Caps-lock enabled"));
    addAction(action, QLineEdit::TrailingPosition);
#endif
}

PasswordEdit::~PasswordEdit()
{
    unregisterHandler();
}

void PasswordEdit::registerHandler()
{
#ifdef QTOX_PLATFORM_EXT
    if (!eventHandler)
        eventHandler = std::make_unique<EventHandler>();
    if (!eventHandler->actions.contains(action))
        eventHandler->actions.append(action);
#endif
}

void PasswordEdit::unregisterHandler()
{
#ifdef QTOX_PLATFORM_EXT
    int idx;

    if (eventHandler && (idx = eventHandler->actions.indexOf(action)) >= 0) {
        eventHandler->actions.remove(idx);
        if (eventHandler->actions.isEmpty()) {
            eventHandler = nullptr;
        }
    }
#endif
}

void PasswordEdit::showEvent(QShowEvent* event)
{
    std::ignore = event;
#ifdef QTOX_PLATFORM_EXT
    action->setVisible(Platform::capsLockEnabled());
#endif
    registerHandler();
}

void PasswordEdit::hideEvent(QHideEvent* event)
{
    std::ignore = event;
    unregisterHandler();
}

PasswordEdit::EventHandler::EventHandler()
{
    QCoreApplication::instance()->installEventFilter(this);
}

PasswordEdit::EventHandler::~EventHandler()
{
    QCoreApplication::instance()->removeEventFilter(this);
}

void PasswordEdit::EventHandler::updateActions()
{
#ifdef QTOX_PLATFORM_EXT
    const bool caps = Platform::capsLockEnabled();

    for (QAction* actionIt : actions)
        actionIt->setVisible(caps);
#endif // QTOX_PLATFORM_EXT
}

bool PasswordEdit::EventHandler::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type()) {
    case QEvent::WindowActivate:
    case QEvent::KeyRelease:
        updateActions();
        break;
    default:
        break;
    }

    return QObject::eventFilter(obj, event);
}
