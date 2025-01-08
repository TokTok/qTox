/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#include "sandboxclient.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wheader-hygiene"
#include "rep_sandbox_replica.h"
#pragma GCC diagnostic pop

#include <QCoreApplication>
#include <QPixmap>
#include <QProcess>
#include <QTimer>

struct SandboxClient::Data
{
    explicit Data(QObject* parent) {}

    QRemoteObjectNode repNode;
    std::unique_ptr<SandboxReplica> sandboxReplica;
    QProcess server;
};

SandboxClient::SandboxClient(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Data>(this))
{
    qDebug() << "Starting sandbox server";
    d->server.start(QCoreApplication::applicationFilePath(), {"--sandbox"}, QIODevice::ReadOnly);

    // Write any output from the server to the console.
    connect(&d->server, &QProcess::readyReadStandardOutput, this,
            [this] { qDebug() << d->server.readAllStandardOutput(); });
    connect(&d->server, &QProcess::readyReadStandardError, this,
            [this] { qWarning() << d->server.readAllStandardError(); });

    // If the server process dies, wait 5 seconds and start it back up.
    auto restartConnection = connect(&d->server, &QProcess::finished, this, [this](int exitCode) {
        qWarning() << "Sandbox server died with exit code" << exitCode;
        QTimer::singleShot(5000, this, [this] {
            d->server.start(QCoreApplication::applicationFilePath(), {"--sandbox"});
        });
    });

    // If our application is about to quit, kill the server process.
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this, restartConnection] {
        disconnect(restartConnection);
        d->server.kill();
    });

    d->repNode.connectToNode(QUrl(QStringLiteral("local:replica")));
    d->sandboxReplica.reset(d->repNode.acquire<SandboxReplica>());

    // Wait for up to 2 seconds for the server to start.
    if (!d->server.waitForStarted(2000)) {
        qWarning() << "Failed to start sandbox server";
        return;
    }

    // Wait for up to 10 seconds for the replica to be acquired.
    if (!d->sandboxReplica->waitForSource(10000)) {
        qWarning() << "Failed to acquire sandbox replica";
    }
}

SandboxClient::~SandboxClient()
{
    d->server.kill();
}

bool SandboxClient::isReady() const
{
    return d->sandboxReplica->isReplicaValid();
}

QPixmap SandboxClient::loadImage(const QByteArray& data)
{
    auto reply = d->sandboxReplica->loadImage(data);
    reply.waitForFinished();
    if (!reply.isFinished()) {
        qWarning() << "Failed to load image from sandbox" << reply.error();
        return {};
    }
    return reply.returnValue();
}
