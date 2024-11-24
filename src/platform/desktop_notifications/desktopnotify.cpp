/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#if DESKTOP_NOTIFICATIONS
#include "desktopnotify.h"

#include "src/persistence/settings.h"

#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN)
// We need to undef signals because libnotify uses it as a parameter name.
#undef signals
#include <libnotify/notification.h>
#include <libnotify/notify.h>

#include <QColorSpace>
#include <QDebug>
#include <QThread>

namespace {

GdkPixbuf* convert(const QPixmap& pixmap)
{
    const QImage image = pixmap.toImage();
    guchar* dataCopy = new guchar[image.sizeInBytes()];
    std::copy(image.bits(), image.bits() + image.sizeInBytes(), dataCopy);
    return gdk_pixbuf_new_from_data(
        dataCopy, GDK_COLORSPACE_RGB, true, 8, image.width(), image.height(), image.bytesPerLine(),
        [](guchar* data, gpointer user_data) {
            std::ignore = user_data;
            delete[] data;
        },
        nullptr);
}

} // namespace

struct NotifyNotificationDeleter
{
    void operator()(NotifyNotification* notification) const
    {
        notify_notification_close(notification, nullptr);
        g_object_unref(notification);
    }
};

struct DesktopNotify::Impl
{
    Impl()
    {
        notify_init("qTox");
    }

    ~Impl()
    {
        // Delete the notification before uninitializing the library.
        lastNotification = nullptr;
        notify_uninit();
    }

    std::unique_ptr<NotifyNotification, NotifyNotificationDeleter> lastNotification;
};

DesktopNotify::DesktopNotify(Settings& settings_)
    : impl{std::make_unique<Impl>()}
    , settings{settings_}
{
}

DesktopNotify::~DesktopNotify() = default;

void DesktopNotify::notifyMessage(const NotificationData& notificationData)
{
    if (!(settings.getNotify() && settings.getDesktopNotify())) {
        return;
    }

    if (impl->lastNotification) {
        notify_notification_update(impl->lastNotification.get(),
                                   notificationData.title.toUtf8().constData(),
                                   notificationData.message.toUtf8().constData(), nullptr);
    } else {
        auto notification = std::unique_ptr<NotifyNotification, NotifyNotificationDeleter>{
            notify_notification_new(notificationData.title.toUtf8().constData(),
                                    notificationData.message.toUtf8().constData(), nullptr)};

        // If a user actively dismisses the notification, we emit the signal. Otherwise, if it times
        // out, we keep on adding to the "number of messages" part of the notification.
        notify_notification_add_action(
            notification.get(), "dismiss", "Dismiss",
            [](NotifyNotification* target, char* action, gpointer user_data) {
                std::ignore = target;
                std::ignore = action;
                DesktopNotify* self = static_cast<DesktopNotify*>(user_data);
                self->impl->lastNotification = nullptr;
                emit self->notificationClosed();
            },
            this, nullptr);

        impl->lastNotification = std::move(notification);
    }

    GdkPixbuf* icon = convert(notificationData.pixmap);
    if (icon != nullptr) {
        notify_notification_set_image_from_pixbuf(impl->lastNotification.get(), icon);
        g_object_unref(icon);
    }

    notify_notification_show(impl->lastNotification.get(), nullptr);
}

#else // Q_OS_MAC || Q_OS_WIN

struct DesktopNotify::Impl
{
    Impl() {}

    ~Impl() {}
};

DesktopNotify::DesktopNotify(Settings& settings_)
    : impl{std::make_unique<Impl>()}
    , settings{settings_}
{
}

DesktopNotify::~DesktopNotify() = default;

void DesktopNotify::notifyMessage(const NotificationData& notificationData)
{
    if (!(settings.getNotify() && settings.getDesktopNotify())) {
        return;
    }
}

#endif // Q_OS_MAC || Q_OS_WIN
#endif // DESKTOP_NOTIFICATIONS
