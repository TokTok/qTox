/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2026 The TokTok team.
 */

#include "desktopnotifybackend.h" // IWYU pragma: associated

#if !QT_CONFIG(dbus)
struct DesktopNotifyBackend::Private
{};

DesktopNotifyBackend::DesktopNotifyBackend(QObject* parent)
    : QObject(parent)
    , d{nullptr}
{
}
DesktopNotifyBackend::~DesktopNotifyBackend() = default;

bool DesktopNotifyBackend::supportsBodyMarkup() const
{
    return false;
}

bool DesktopNotifyBackend::showMessage(const QString& title, const QString& message,
                                       const QString& category, const QPixmap& pixmap)
{
    std::ignore = title;
    std::ignore = message;
    std::ignore = category;
    std::ignore = pixmap;
    // Always fail, fall back to QSystemTrayIcon.
    return false;
}
#endif // !QT_CONFIG(dbus)
