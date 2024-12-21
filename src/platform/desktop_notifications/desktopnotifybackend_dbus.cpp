/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#include "desktopnotifybackend.h"

#include <QDebug>

#include <tuple> // std::ignore

#if QT_CONFIG(dbus)
#include <QApplication>
#include <QBuffer>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QMutex>
#include <QMutexLocker>
#include <QPixmap>
#include <QTemporaryFile>

namespace {
// TODO(iphydf): Make this a setting.
constexpr bool QTOX_NOTIFY_TEMP_FILE = false;

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

QDebug& operator<<(QDebug& dbg, const NotificationSpecVersion& version)
{
    dbg.nospace() << version.major << '.' << version.minor;
    return dbg.space();
}

/**
 * Almost certainly returns 1.2, but we can support old systems here, too.
 *
 * https://specifications.freedesktop.org/notification-spec/1.2/protocol.html#command-get-server-information
 */
NotificationSpecVersion getSpecVersion(QDBusInterface& notifyInterface)
{
    if (!notifyInterface.isValid()) {
        return {0, 0};
    }

    const auto res = notifyInterface.call(QStringLiteral("GetServerInformation"));
    if (!res.errorMessage().isEmpty()) {
        qWarning() << "Failed to get notification server information:" << res.errorMessage();
        return {0, 0};
    }

    // QList(QVariant(QString, "gnome-shell"), QVariant(QString, "GNOME"), QVariant(QString, "46.3.1"), QVariant(QString, "1.2"))
    const auto version = res.arguments().value(3).toString().split(QLatin1Char('.'));
    return {version.value(0).toInt(), version.value(1).toInt()};
}

QStringList getCapabilities(QDBusInterface& notifyInterface)
{
    if (!notifyInterface.isValid()) {
        return {};
    }

    const auto res = notifyInterface.call(QStringLiteral("GetCapabilities"));
    if (!res.errorMessage().isEmpty()) {
        qWarning() << "Failed to get notification capabilities:" << res.errorMessage();
        return {};
    }

    // QList(QVariant(QStringList, QList("actions", "body", "body-markup", "icon-static", "persistence", "sound")))
    return res.arguments().value(0).toStringList();
}

// https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Notification.html#org-freedesktop-portal-notification-addnotification
//
// Implements the "bytes" (ay) variant of "icon" (v).
struct DBusPortalImage
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

QDBusArgument& operator<<(QDBusArgument& argument, const DBusPortalImage& image)
{
    argument.beginStructure();
    argument << QStringLiteral("bytes");
    // Must be QDBusVariant because the signature is "v" (variant).
    argument << QDBusVariant(image.data());
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, DBusPortalImage& image)
{
    QString type;
    argument.beginStructure();
    argument >> type;
    if (type == QStringLiteral("bytes")) {
        // Unpack the DBus variant.
        QDBusVariant variant;
        argument >> variant;
        const QByteArray data = variant.variant().value<QByteArray>();
        image = DBusPortalImage{QImage::fromData(data, "PNG")};
    }
    argument.endStructure();
    return argument;
}

// https://specifications.freedesktop.org/notification-spec/latest/icons-and-images.html#icons-and-images-formats
//
// image-data (iiibiiay)
struct DBusNotifyImage
{
    QImage image;
};

QDBusArgument& operator<<(QDBusArgument& argument, const DBusNotifyImage& image)
{
    const QImage rgba = image.image.convertToFormat(QImage::Format_RGBA8888);
    argument.beginStructure();
    argument << rgba.width();
    argument << rgba.height();
    argument << rgba.width() * 4;
    argument << true;
    argument << 8;
    argument << 4;
    argument << QByteArray(reinterpret_cast<const char*>(rgba.bits()), rgba.sizeInBytes());
    argument.endStructure();
    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, DBusNotifyImage& image)
{
    int width;
    int height;
    int rowStride;
    bool hasAlpha;
    int bitsPerSample;
    int channels;
    QByteArray data;
    argument.beginStructure();
    argument >> width;
    argument >> height;
    argument >> rowStride;
    argument >> hasAlpha;
    argument >> bitsPerSample;
    argument >> channels;
    argument >> data;
    argument.endStructure();
    image = DBusNotifyImage{QImage{
        reinterpret_cast<const uchar*>(data.constData()),
        width,
        height,
        rowStride,
        channels == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888,
    }};
    return argument;
}

/**
 * @brief Get the name of the image data field for the Notify hints map.
 *
 * Has no use in the AddNotification method of the Portal.
 */
QString imageDataFieldName(const NotificationSpecVersion& specVersion)
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

/**
 * @brief Create a temporary file for the notification icon.
 *
 * We always keep exactly 1 of these around to avoid creating lots of temporary files,
 * potentially filling up the disk (if people change their avatar a lot and send one message
 * after each change). This means it can happen that the file is deleted before the notification
 * is shown, but that's fine because the next notification will immediately override the
 * failed one.
 */
const QFile* getIconFile(const QImage& image, QMutex& iconMutex,
                         std::unique_ptr<QTemporaryFile>& iconFile)
{
    QMutexLocker<QMutex> lock(&iconMutex);
    const qint64 iconKey = image.cacheKey();
    iconFile = std::make_unique<QTemporaryFile>(
        QStringList{QDir::tempPath(),
                    QStringLiteral("%1-notification-%2-XXXXXX.png")
                        .arg(QCoreApplication::applicationName(), QString::number(iconKey))}
            .join(QDir::separator()));
    if (!iconFile->open()) {
        return nullptr;
    }
    image.save(iconFile.get(), "PNG");
    iconFile->close();
    return iconFile.get();
}

/**
 * @brief Create either a temporary file or in-memory image for the notification icon.
 *
 * @param pixmap The icon to show in the notification.
 * @param specVersion The notification spec version.
 * @param capabilities The notification capabilities (if icon-* is not supported, we don't send the icon hint).
 * @param iconMutex The mutex to protect the icon file creation.
 * @param iconFile The temporary file to store the icon in.
 * @param useTempFile Whether to use a temporary file or in-memory image.
 *
 * @return The icon hint to add to the notification hints map.
 */
QVariantMap getIconHint(const QPixmap& pixmap, const NotificationSpecVersion& specVersion,
                        const QStringList& capabilities, QMutex& iconMutex,
                        std::unique_ptr<QTemporaryFile>& iconFile, bool useTempFile)
{
    // https://specifications.freedesktop.org/notification-spec/1.2/protocol.html#id-1.10.3.2.5
    if (!(capabilities.contains(QStringLiteral("icon-multi"))
          || capabilities.contains(QStringLiteral("icon-static")))) {
        // Icons not supported.
        return {};
    }

    if (useTempFile) {
        const auto* tempFile = getIconFile(pixmap.toImage(), iconMutex, iconFile);
        if (tempFile != nullptr) {
            return {{QStringLiteral("image-path"), tempFile->fileName()}};
        }

        // Fall back to in-memory image sending.
        qWarning() << "Failed to create temporary file for notification image; attempting "
                      "in-memory image sending";
    }

    return {{imageDataFieldName(specVersion), QVariant::fromValue(DBusNotifyImage{pixmap.toImage()})}};
}
} // namespace

Q_DECLARE_METATYPE(DBusPortalImage)
Q_DECLARE_METATYPE(DBusNotifyImage)

struct DesktopNotifyBackend::Private
{
    QDBusInterface notifyInterface;
    QDBusInterface notifyPortalInterface;
    const NotificationSpecVersion specVersion;
    const QStringList capabilities;
    QMutex iconMutex;
    std::unique_ptr<QTemporaryFile> iconFile;
    uint id = 0;

    explicit Private(QObject* parent)
        : notifyInterface(NOTIFY_DBUS_NAME, NOTIFY_DBUS_CORE_OBJECT, NOTIFY_DBUS_CORE_INTERFACE,
                          QDBusConnection::sessionBus(), parent)
        , notifyPortalInterface(NOTIFY_PORTAL_DBUS_NAME, NOTIFY_PORTAL_DBUS_CORE_OBJECT,
                                NOTIFY_PORTAL_DBUS_CORE_INTERFACE, QDBusConnection::sessionBus(),
                                parent)
        , specVersion(getSpecVersion(notifyInterface))
        , capabilities(getCapabilities(notifyInterface))
    {
        qDebug() << "Notification spec version:" << specVersion << "capabilities:" << capabilities;
        qDBusRegisterMetaType<DBusPortalImage>();
        qDBusRegisterMetaType<DBusNotifyImage>();
        // Set a 1 second timeout for all DBus calls. In case these calls freeze
        // the application, it'll "only" be for a second.
        notifyInterface.setTimeout(1000);
        notifyPortalInterface.setTimeout(1000);
    }
};

DesktopNotifyBackend::DesktopNotifyBackend(QObject* parent)
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

DesktopNotifyBackend::~DesktopNotifyBackend() = default;

bool DesktopNotifyBackend::showMessage(const QString& title, const QString& message,
                                       const QPixmap& pixmap)
{
    // Try Notify first.
    if (d->notifyInterface.isValid()) {
        QVariantMap hints{
            {QStringLiteral("action-icons"), true},
            {QStringLiteral("category"), QStringLiteral("im.received")},
            {QStringLiteral("sender-pid"),
             QVariant::fromValue<quint64>(QCoreApplication::applicationPid())},
        };
        hints.insert(getIconHint(pixmap, d->specVersion, d->capabilities, d->iconMutex, d->iconFile,
                                 QTOX_NOTIFY_TEMP_FILE));
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
            qWarning() << "Notification could not be shown via Notifications:" << res.errorMessage();
            return false;
        }
        d->id = res.arguments().value(0).toUInt();
        return true;
    }

    // Otherwise, try the Portal.
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
                {QStringLiteral("icon"), QVariant::fromValue(DBusPortalImage{pixmap.toImage()})},
                // default-action (s)
                {QStringLiteral("default-action"), QStringLiteral("default")},
            });
        if (res.errorMessage().isEmpty()) {
            return true;
        }
        qWarning() << "Notification could not be shown via Portal:" << res.errorMessage();
    }

    return false;
}
#endif // QT_CONFIG(dbus)

// TODO(iphydf): These should probably be more private, but then Private needs to be a QObject.
void DesktopNotifyBackend::notificationActionInvoked(QString actionKey, QString actionValue)
{
    std::ignore = actionKey;
    std::ignore = actionValue;
    emit messageClicked();
}

void DesktopNotifyBackend::notificationActionInvoked(uint actionKey, QString actionValue)
{
    std::ignore = actionKey;
    std::ignore = actionValue;
    emit messageClicked();
}
