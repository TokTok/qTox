/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#include "desktopnotify_dbus.h"

#include <QDebug>
#ifdef WITH_DBUS
#include <QApplication>
#include <QBuffer>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusVariant>
#include <QFile>
#include <QImage>
#include <QPixmap>

namespace {
const QString NOTIFY_DBUS_NAME = QStringLiteral("org.freedesktop.Notifications");
const QString NOTIFY_DBUS_CORE_INTERFACE = QStringLiteral("org.freedesktop.Notifications");
const QString NOTIFY_DBUS_CORE_OBJECT = QStringLiteral("/org/freedesktop/Notifications");

const QString NOTIFY_PORTAL_DBUS_NAME = QStringLiteral("org.freedesktop.portal.Desktop");
const QString NOTIFY_PORTAL_DBUS_CORE_INTERFACE =
    QStringLiteral("org.freedesktop.portal.Notification");
const QString NOTIFY_PORTAL_DBUS_CORE_OBJECT = QStringLiteral("/org/freedesktop/portal/desktop");

struct NotificationSpecVersion
{
    int major;
    int minor;

    bool operator>=(const NotificationSpecVersion& other) const
    {
        return major > other.major || (major == other.major && minor >= other.minor);
    }
};

QDebug operator<<(QDebug dbg, const NotificationSpecVersion& version)
{
    return dbg.nospace() << version.major << '.' << version.minor;
}

// https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Notification.html#org-freedesktop-portal-notification-addnotification
//
// Implements the "bytes" (ay) variant of "icon" (v).
struct DBusImage
{
    QByteArray data() const
    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        return buffer.data();
    }

    QImage image;
};

QDBusArgument& operator<<(QDBusArgument& argument, const DBusImage& image)
{
    argument.beginStructure();
    argument << QStringLiteral("bytes");
    // Must be QDBusVariant because the signature is "v" (variant).
    argument << QDBusVariant(image.data());
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, DBusImage& image)
{
    QString type;
    argument.beginStructure();
    argument >> type;
    if (type == QStringLiteral("bytes")) {
        // Unpack the DBus variant.
        QDBusVariant variant;
        argument >> variant;
        const QByteArray data = variant.variant().value<QByteArray>();
        image = DBusImage{QImage::fromData(data, "PNG")};
    }
    argument.endStructure();
    return argument;
}
} // namespace

Q_DECLARE_METATYPE(DBusImage)

class DesktopNotifyDBus::Private
{
public:
    QDBusInterface notifyInterface;
    QDBusInterface notifyPortalInterface;
    const NotificationSpecVersion specVersion;
    uint id = 0;

    explicit Private(QObject* parent)
        : notifyInterface(NOTIFY_DBUS_NAME, NOTIFY_DBUS_CORE_OBJECT, NOTIFY_DBUS_CORE_INTERFACE,
                          QDBusConnection::sessionBus(), parent)
        , notifyPortalInterface(NOTIFY_PORTAL_DBUS_NAME, NOTIFY_PORTAL_DBUS_CORE_OBJECT,
                                NOTIFY_PORTAL_DBUS_CORE_INTERFACE, QDBusConnection::sessionBus(),
                                parent)
        , specVersion(getSpecVersion())
    {
        qDebug() << "Notification spec version:" << specVersion;
        qDBusRegisterMetaType<DBusImage>();
    }

    QString imageDataFieldName() const
    {
        // https://specifications.freedesktop.org/notification-spec/1.2/hints.html
        if (specVersion >= NotificationSpecVersion{1, 2}) {
            return QStringLiteral("image-data");
        }
        if (specVersion >= NotificationSpecVersion{1, 1}) {
            return QStringLiteral("image_data");
        }
        return QStringLiteral("icon_data");
    }

    NotificationSpecVersion getSpecVersion()
    {
        if (!notifyInterface.isValid()) {
            return {0, 0};
        }

        const auto res = notifyInterface.call(QStringLiteral("GetServerInformation"));
        if (!res.errorMessage().isEmpty()) {
            qWarning() << "Failed to get notification server information:" << res.errorMessage();
            return {0, 0};
        }

        qDebug() << "Notification server information:" << res.arguments();
        // QList(QVariant(QString, "gnome-shell"), QVariant(QString, "GNOME"), QVariant(QString, "46.3.1"), QVariant(QString, "1.2"))
        const auto version = res.arguments().value(3).toString().split(QLatin1Char('.'));
        return {version.value(0).toInt(), version.value(1).toInt()};
    }
};

DesktopNotifyDBus::DesktopNotifyDBus(QObject* parent)
    : QObject(parent)
    , d{std::make_unique<Private>(this)}
{
    if (d->notifyPortalInterface.isValid()) {
        QDBusConnection::sessionBus().connect(
            // org.freedesktop.portal.Notification::ActionInvoked
            NOTIFY_PORTAL_DBUS_NAME, NOTIFY_PORTAL_DBUS_CORE_OBJECT, NOTIFY_PORTAL_DBUS_CORE_INTERFACE,
            QStringLiteral("ActionInvoked"), this, SLOT(notificationActionInvoked(QString, QString)));
    }
    if (d->notifyInterface.isValid()) {
        QDBusConnection::sessionBus().connect(
            // org.freedesktop.Notifications::ActionInvoked
            NOTIFY_DBUS_NAME, NOTIFY_DBUS_CORE_OBJECT, NOTIFY_DBUS_CORE_INTERFACE,
            QStringLiteral("ActionInvoked"), this, SLOT(notificationActionInvoked(uint, QString)));
    }
}

DesktopNotifyDBus::~DesktopNotifyDBus() = default;

bool DesktopNotifyDBus::showMessage(const QString& title, const QString& message, const QPixmap& pixmap)
{
    // Try Portal first.
    if (d->notifyPortalInterface.isValid()) {
        const QString notificationId = QStringLiteral("qtox-notification-%1").arg(d->id);

        // https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Notification.html#org-freedesktop-portal-notification-addnotification
        const auto res = d->notifyPortalInterface.call( //
            QStringLiteral("AddNotification"),
            // IN id s
            notificationId,
            // IN notification a{sv}
            QVariantMap{
                // title (s)
                {QStringLiteral("title"), title},
                // body (s)
                {QStringLiteral("body"), message},
                // icon (v)
                {QStringLiteral("icon"), QVariant::fromValue(DBusImage{pixmap.toImage()})},
                // default-action (s)
                {QStringLiteral("default-action"), QStringLiteral("default")},
            });
        if (res.errorMessage().isEmpty()) {
            return true;
        }
        qWarning() << "Notification could not be shown via Portal:" << res.errorMessage();
    }

    // Fallback to Notify.
    if (d->notifyInterface.isValid()) {
        const QVariantMap hints{
            {QStringLiteral("action-icons"), true},
            {QStringLiteral("category"), QStringLiteral("im.received")},
            {QStringLiteral("sender-pid"),
             QVariant::fromValue<quint64>(QCoreApplication::applicationPid())},
        };
        // https://specifications.freedesktop.org/notification-spec/1.2/protocol.html#command-notify
        const auto res = d->notifyInterface.call( //
            QStringLiteral("Notify"),
            // app_name
            QApplication::applicationName(),
            // replaces_id
            d->id,
            // app_icon
            QStringLiteral("dialog-password"),
            // summary
            title,
            // body
            message,
            // actions
            QStringList{"default", "Close"},
            // hints
            hints,
            // expire_timeout
            -1);
        if (!res.errorMessage().isEmpty()) {
            qWarning() << "Notification could not be shown via DBus:" << res.errorMessage();
            return false;
        }
        d->id = res.arguments().value(0).toUInt();
        return true;
    }

    return false;
}
#else
#include <utility>

class DesktopNotifyDBus::Private
{};

DesktopNotifyDBus::DesktopNotifyDBus(QObject* parent)
    : QObject(parent)
    , d{nullptr}
{
}
DesktopNotifyDBus::~DesktopNotifyDBus() = default;

bool DesktopNotifyDBus::showMessage(const QString& title, const QString& message, const QPixmap& pixmap)
{
    std::ignore = title;
    std::ignore = message;
    std::ignore = pixmap;
    // Always fail, fall back to QSystemTrayIcon.
    return false;
}
#endif

void DesktopNotifyDBus::notificationActionInvoked(QString actionKey, QString actionValue)
{
    qDebug() << "Notification action invoked:" << actionKey << actionValue;
    emit messageClicked();
}

void DesktopNotifyDBus::notificationActionInvoked(uint actionKey, QString actionValue)
{
    qDebug() << "Notification action invoked:" << actionKey << actionValue;
    emit messageClicked();
}
