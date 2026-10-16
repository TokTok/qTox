/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#include "src/platform/desktop_notifications/desktopnotifybackend.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QPixmap>
#include <QtTest/QtTest>

#include <memory>

namespace {
const QString serviceName = QStringLiteral("org.freedesktop.Notifications");
const QString objectPath = QStringLiteral("/org/freedesktop/Notifications");
const QString markupBody = QStringLiteral("<img src=\"/tmp/x.png\"/> & <b>hi</b>");
} // namespace

class FakeNotificationServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")

public:
    explicit FakeNotificationServer(QStringList capabilities_)
        : capabilities{std::move(capabilities_)}
    {
    }

    QString lastBody;

public slots:
    QStringList GetCapabilities()
    {
        return capabilities;
    }

    QString GetServerInformation(QString& vendor, QString& version, QString& specVersion)
    {
        vendor = QStringLiteral("qTox");
        version = QStringLiteral("1.0");
        specVersion = QStringLiteral("1.2");
        return QStringLiteral("fake");
    }

    uint Notify(const QString& appName, uint replacesId, const QString& appIcon,
                const QString& summary, const QString& body, const QStringList& actions,
                const QVariantMap& hints, int expireTimeout)
    {
        std::ignore = appName;
        std::ignore = replacesId;
        std::ignore = appIcon;
        std::ignore = summary;
        std::ignore = actions;
        std::ignore = hints;
        std::ignore = expireTimeout;
        lastBody = body;
        return 1;
    }

private:
    const QStringList capabilities;
};

class TestDesktopNotifyBackend : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();
    void escapesBodyWithMarkupSupport();
    void keepsBodyWithoutMarkupSupport();

private:
    FakeNotificationServer* startServer(const QStringList& capabilities);

    std::unique_ptr<FakeNotificationServer> server;
};

void TestDesktopNotifyBackend::initTestCase()
{
    if (qEnvironmentVariableIsEmpty("DBUS_SESSION_BUS_ADDRESS")
        || !QDBusConnection::sessionBus().isConnected()) {
        QSKIP("No D-Bus session bus (run with dbus-run-session)");
    }
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName)) {
        QSKIP("A real notification server is running on this session bus");
    }
}

void TestDesktopNotifyBackend::cleanup()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.unregisterObject(objectPath);
    bus.unregisterService(serviceName);
    server.reset();
}

FakeNotificationServer* TestDesktopNotifyBackend::startServer(const QStringList& capabilities)
{
    server = std::make_unique<FakeNotificationServer>(capabilities);
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(objectPath, server.get(), QDBusConnection::ExportAllSlots)
        || !bus.registerService(serviceName)) {
        return nullptr;
    }
    return server.get();
}

void TestDesktopNotifyBackend::escapesBodyWithMarkupSupport()
{
    const FakeNotificationServer* fake = startServer({"body", "body-markup"});
    QVERIFY(fake != nullptr);

    DesktopNotifyBackend backend{nullptr};
    QVERIFY(backend.supportsBodyMarkup());
    QVERIFY(backend.showMessage(QStringLiteral("title"), markupBody, QStringLiteral("im.received"),
                                QPixmap()));
    QCOMPARE(fake->lastBody, markupBody.toHtmlEscaped());
}

void TestDesktopNotifyBackend::keepsBodyWithoutMarkupSupport()
{
    const FakeNotificationServer* fake = startServer({"body"});
    QVERIFY(fake != nullptr);

    DesktopNotifyBackend backend{nullptr};
    QVERIFY(!backend.supportsBodyMarkup());
    QVERIFY(backend.showMessage(QStringLiteral("title"), markupBody, QStringLiteral("im.received"),
                                QPixmap()));
    QCOMPARE(fake->lastBody, markupBody);
}

QTEST_MAIN(TestDesktopNotifyBackend)
#include "desktopnotifybackend_test.moc"
