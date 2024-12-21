/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2018-2020 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>

#include "src/core/dhtserver.h"
#include "src/model/ibootstraplistgenerator.h"

class QNetworkReply;
class Paths;

class BootstrapNodeUpdater : public QObject, public IBootstrapListGenerator
{
    Q_OBJECT
public:
    explicit BootstrapNodeUpdater(const QNetworkProxy& proxy_, Paths& paths_,
                                  QObject* parent = nullptr);
    QList<DhtServer> getBootstrapNodes() const override;
    void requestBootstrapNodes();
    static QList<DhtServer> loadDefaultBootstrapNodes();

signals:
    void availableBootstrapNodes(QList<DhtServer> nodes);

private slots:
    void onRequestComplete(QNetworkReply* reply);

private:
    QNetworkProxy proxy;
    QNetworkAccessManager nam;
    Paths& paths;
};
